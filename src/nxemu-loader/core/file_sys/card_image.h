// SPDX-FileCopyrightText: Copyright 2018 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <array>
#include <memory>
#include <vector>
#include "yuzu_common/common_types.h"
#include "yuzu_common/swap.h"
#include "core/file_sys/vfs/vfs.h"

enum class LoaderResultStatus : uint16_t;

namespace FileSys {

enum class GamecardSize : u8 {
    S_1GB = 0xFA,
    S_2GB = 0xF8,
    S_4GB = 0xF0,
    S_8GB = 0xE0,
    S_16GB = 0xE1,
    S_32GB = 0xE2,
};

struct GamecardInfo {
    u64_le firmware_version;
    u32_le access_control_flags;
    u32_le read_wait_time1;
    u32_le read_wait_time2;
    u32_le write_wait_time1;
    u32_le write_wait_time2;
    u32_le firmware_mode;
    u32_le cup_version;
    std::array<u8, 4> reserved1;
    u64_le update_partition_hash;
    u64_le cup_id;
    std::array<u8, 0x38> reserved2;
};
static_assert(sizeof(GamecardInfo) == 0x70, "GamecardInfo has incorrect size.");

struct GamecardHeader {
    std::array<u8, 0x100> signature;
    u32_le magic;
    u32_le secure_area_start;
    u32_le backup_area_start;
    u8 kek_index;
    GamecardSize size;
    u8 header_version;
    u8 flags;
    u64_le package_id;
    u64_le valid_data_end;
    u128 info_iv;
    u64_le hfs_offset;
    u64_le hfs_size;
    std::array<u8, 0x20> hfs_header_hash;
    std::array<u8, 0x20> initial_data_hash;
    u32_le secure_mode_flag;
    u32_le title_key_flag;
    u32_le key_flag;
    u32_le normal_area_end;
    GamecardInfo info;
};
static_assert(sizeof(GamecardHeader) == 0x200, "GamecardHeader has incorrect size.");

enum class XCIPartition : u8 { Update, Normal, Secure, Logo };

class XCI : public ReadOnlyVfsDirectory {
public:
    explicit XCI(VirtualFile file, u64 program_id = 0, size_t program_index = 0);
    ~XCI() override;

    LoaderResultStatus GetStatus() const;
    LoaderResultStatus GetProgramNCAStatus() const;

    u8 GetFormatVersion();

    VirtualDir GetPartition(XCIPartition partition);
    std::vector<VirtualDir> GetPartitions();

    VirtualDir GetSecurePartition();
    VirtualDir GetNormalPartition();
    VirtualDir GetUpdatePartition();
    VirtualDir GetLogoPartition();

    VirtualFile GetPartitionRaw(XCIPartition partition) const;
    VirtualFile GetSecurePartitionRaw() const;
    VirtualFile GetStoragePartition0() const;
    VirtualFile GetStoragePartition1() const;
    VirtualFile GetNormalPartitionRaw() const;
    VirtualFile GetUpdatePartitionRaw() const;
    VirtualFile GetLogoPartitionRaw() const;

    u64 GetProgramTitleID() const;
    std::vector<u64> GetProgramTitleIDs() const;
    u32 GetSystemUpdateVersion();
    u64 GetSystemUpdateTitleID() const;

    std::vector<VirtualFile> GetFiles() const override;

    std::vector<VirtualDir> GetSubdirectories() const override;

    std::string GetName() const override;

    VirtualDir GetParentDirectory() const override;

    // Creates a directory that contains all the NCAs in the gamecard
    VirtualDir ConcatenatedPseudoDirectory();

private:
    LoaderResultStatus TryReadHeader();

    VirtualFile file;
    GamecardHeader header{};

    LoaderResultStatus status;
    LoaderResultStatus program_nca_status;

    std::vector<VirtualDir> partitions;
    std::vector<VirtualFile> partitions_raw;

    u64 update_normal_partition_end;
};
} // namespace FileSys
