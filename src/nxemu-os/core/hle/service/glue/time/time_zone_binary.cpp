// SPDX-FileCopyrightText: Copyright 2023 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include "core/core.h"
#include "core/hle/service/filesystem/filesystem.h"
#include "core/hle/service/glue/time/time_zone_binary.h"
#include "core/file_sys/filesystem_interfaces.h"
#include <nxemu-module-spec/system_loader.h>

namespace Service::Glue::Time {
namespace {
constexpr u64 TimeZoneBinaryId = 0x10000000000080E;

static IVirtualDirectoryPtr g_time_zone_binary_romfs{};
static Result g_time_zone_binary_mount_result{ResultUnknown};
static std::vector<u8> g_time_zone_scratch_space(0x2800, 0);

Result TimeZoneReadBinary(Core::System& system, uint32_t & out_read_size, std::span<u8> out_buffer, size_t out_buffer_size,
                          std::string_view path) {
    R_UNLESS(g_time_zone_binary_mount_result == ResultSuccess, g_time_zone_binary_mount_result);

    IVirtualFilePtr vfs_file{ g_time_zone_binary_romfs->GetFileRelative(path.data()) };
    R_UNLESS(vfs_file, ResultUnknown);

    auto file_size{ vfs_file->GetSize() };
    R_UNLESS(file_size > 0, ResultUnknown);

    R_UNLESS(file_size <= out_buffer_size, Service::PSC::Time::ResultFailed);

    out_read_size = vfs_file->ReadBytes(out_buffer.data(), file_size, 0);
    R_UNLESS(out_read_size > 0, ResultUnknown);

    R_SUCCEED();
}
} // namespace

void ResetTimeZoneBinary() {
    g_time_zone_binary_mount_result = ResultUnknown;
    g_time_zone_scratch_space.clear();
    g_time_zone_scratch_space.resize(0x2800, 0);
}

Result MountTimeZoneBinary(Core::System& system) 
{
    ResetTimeZoneBinary();

    ISystemloader & loader = system.GetSystemloader();
    auto& fsc{ system.GetFileSystemController() };
    auto* bis_system = fsc.GetSystemNANDContents();
    R_UNLESS(bis_system, ResultUnknown);
    FileSysNCAPtr nca(bis_system->GetEntry(TimeZoneBinaryId, LoaderContentRecordType::Data));

    if (nca) 
    {        
        g_time_zone_binary_romfs = IVirtualDirectoryPtr(nca->GetRomFS()->ExtractRomFS());
    }

    if (g_time_zone_binary_romfs) 
    {
        // Validate that the romfs is readable, using invalid firmware keys can cause this to get
        // set but the files to be garbage. In that case, we want to hit the next path and
        // synthesise them instead.
        g_time_zone_binary_mount_result = ResultSuccess;
        Service::PSC::Time::LocationName name{ "Etc/GMT" };
        if (!IsTimeZoneBinaryValid(system, name))
        {
            ResetTimeZoneBinary();
        }
    }

    if (!g_time_zone_binary_romfs) 
    {
        g_time_zone_binary_romfs = IVirtualFilePtr(loader.SynthesizeSystemArchive(TimeZoneBinaryId))->ExtractRomFS();
    }

    R_UNLESS(g_time_zone_binary_romfs, ResultUnknown);

    g_time_zone_binary_mount_result = ResultSuccess;
    R_SUCCEED();
}

void GetTimeZoneBinaryListPath(std::string& out_path) {
    if (g_time_zone_binary_mount_result != ResultSuccess) {
        return;
    }
    // out_path = fmt::format("{}:/binaryList.txt", "TimeZoneBinary");
    out_path = "/binaryList.txt";
}

void GetTimeZoneBinaryVersionPath(std::string& out_path) {
    if (g_time_zone_binary_mount_result != ResultSuccess) {
        return;
    }
    // out_path = fmt::format("{}:/version.txt", "TimeZoneBinary");
    out_path = "/version.txt";
}

void GetTimeZoneZonePath(std::string& out_path, const Service::PSC::Time::LocationName& name) {
    if (g_time_zone_binary_mount_result != ResultSuccess) {
        return;
    }
    // out_path = fmt::format("{}:/zoneinfo/{}", "TimeZoneBinary", name);
    out_path = fmt::format("/zoneinfo/{}", name.data());
}

bool IsTimeZoneBinaryValid(Core::System & system, const Service::PSC::Time::LocationName& name) {
    std::string path{};
    GetTimeZoneZonePath(path, name);

    IVirtualFilePtr vfs_file(g_time_zone_binary_romfs->GetFileRelative(path.c_str()));
    if (!vfs_file) 
    {
        LOG_INFO(Service_Time, "Could not find timezone file {}", path);
        return false;
    }
    return vfs_file->GetSize() != 0;
}

u32 GetTimeZoneCount(Core::System & system) {
    std::string path{};
    GetTimeZoneBinaryListPath(path);

    uint32_t bytes_read{};
    if (TimeZoneReadBinary(system, bytes_read, g_time_zone_scratch_space, 0x2800, path) != ResultSuccess) {
        return 0;
    }
    if (bytes_read == 0) {
        return 0;
    }

    auto chars = std::span(reinterpret_cast<char*>(g_time_zone_scratch_space.data()), bytes_read);
    u32 count{};
    for (auto chr : chars) {
        if (chr == '\n') {
            count++;
        }
    }
    return count;
}

Result GetTimeZoneVersion(Core::System & system, Service::PSC::Time::RuleVersion& out_rule_version) {
    std::string path{};
    GetTimeZoneBinaryVersionPath(path);

    auto rule_version_buffer{std::span(reinterpret_cast<u8*>(&out_rule_version),
                                       sizeof(Service::PSC::Time::RuleVersion))};
    uint32_t bytes_read{};
    R_TRY(TimeZoneReadBinary(system, bytes_read, rule_version_buffer, rule_version_buffer.size_bytes(),
                             path));

    rule_version_buffer[bytes_read] = 0;
    R_SUCCEED();
}

Result GetTimeZoneRule(Core::System & system, std::span<const u8>& out_rule, size_t& out_rule_size,
                       const Service::PSC::Time::LocationName& name) {
    std::string path{};
    GetTimeZoneZonePath(path, name);

    uint32_t bytes_read{};
    R_TRY(TimeZoneReadBinary(system, bytes_read, g_time_zone_scratch_space,
                             g_time_zone_scratch_space.size(), path));

    out_rule = std::span(g_time_zone_scratch_space.data(), bytes_read);
    out_rule_size = bytes_read;
    R_SUCCEED();
}

Result GetTimeZoneLocationList(Core::System& system, u32& out_count,
                               std::span<Service::PSC::Time::LocationName> out_names,
                               size_t max_names, u32 index) {
    std::string path{};
    GetTimeZoneBinaryListPath(path);

    uint32_t bytes_read{};
    R_TRY(TimeZoneReadBinary(system, bytes_read, g_time_zone_scratch_space,
                             g_time_zone_scratch_space.size(), path));

    out_count = 0;
    R_SUCCEED_IF(bytes_read == 0);

    Service::PSC::Time::LocationName current_name{};
    size_t current_name_len{};
    std::span<const u8> chars{g_time_zone_scratch_space};
    u32 name_count{};

    for (auto chr : chars) {
        if (chr == '\r') {
            continue;
        }

        if (chr == '\n') {
            if (name_count >= index) {
                out_names[out_count] = current_name;
                out_count++;
                if (out_count >= max_names) {
                    break;
                }
            }
            name_count++;
            current_name_len = 0;
            current_name = {};
            continue;
        }

        if (chr == '\0') {
            break;
        }

        R_UNLESS(current_name_len <= current_name.size() - 2, Service::PSC::Time::ResultFailed);

        current_name[current_name_len++] = chr;
    }

    R_SUCCEED();
}

} // namespace Service::Glue::Time
