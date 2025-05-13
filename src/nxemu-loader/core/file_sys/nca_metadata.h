// SPDX-FileCopyrightText: Copyright 2018 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <array>
#include <vector>
#include "yuzu_common/common_funcs.h"
#include "yuzu_common/common_types.h"
#include "yuzu_common/swap.h"
#include "core/file_sys/vfs/vfs_types.h"
#include "nxemu-module-spec/system_loader.h"

namespace FileSys {
class CNMT;

struct CNMTHeader;
struct OptionalHeader;

struct ContentRecord {
    std::array<u8, 0x20> hash;
    std::array<u8, 0x10> nca_id;
    std::array<u8, 0x6> size;
    LoaderContentRecordType type;
    INSERT_PADDING_BYTES(1);
};
static_assert(sizeof(ContentRecord) == 0x38, "ContentRecord has incorrect size.");

constexpr ContentRecord EMPTY_META_CONTENT_RECORD{{}, {}, {}, LoaderContentRecordType::Meta, {}};

struct MetaRecord {
    u64_le title_id;
    u32_le title_version;
    LoaderTitleType type;
    u8 install_byte;
    INSERT_PADDING_BYTES(2);
};
static_assert(sizeof(MetaRecord) == 0x10, "MetaRecord has incorrect size.");

struct OptionalHeader {
    u64_le title_id;
    u64_le minimum_version;
};
static_assert(sizeof(OptionalHeader) == 0x10, "OptionalHeader has incorrect size.");

struct CNMTHeader {
    u64_le title_id;
    u32_le title_version;
    LoaderTitleType type;
    u8 reserved;
    u16_le table_offset;
    u16_le number_content_entries;
    u16_le number_meta_entries;
    u8 attributes;
    std::array<u8, 2> reserved2;
    u8 is_committed;
    u32_le required_download_system_version;
    std::array<u8, 4> reserved3;
};
static_assert(sizeof(CNMTHeader) == 0x20, "CNMTHeader has incorrect size.");

// A class representing the format used by NCA metadata files, typically named {}.cnmt.nca or
// meta0.ncd. These describe which NCA's belong with which titles in the registered cache.
class CNMT {
public:
    explicit CNMT(VirtualFile file);
    CNMT(CNMTHeader header_, OptionalHeader opt_header_,
         std::vector<ContentRecord> content_records_, std::vector<MetaRecord> meta_records_);
    ~CNMT();

    const CNMTHeader& GetHeader() const;
    u64 GetTitleID() const;
    u32 GetTitleVersion() const;
    LoaderTitleType GetType() const;

    const std::vector<ContentRecord>& GetContentRecords() const;
    const std::vector<MetaRecord>& GetMetaRecords() const;

    bool UnionRecords(const CNMT& other);
    std::vector<u8> Serialize() const;

private:
    CNMTHeader header;
    OptionalHeader opt_header;
    std::vector<ContentRecord> content_records;
    std::vector<MetaRecord> meta_records;

    // TODO(DarkLordZach): According to switchbrew, for Patch-type there is additional data
    // after the table. This is not documented, unfortunately.
};

} // namespace FileSys
