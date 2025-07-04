// SPDX-FileCopyrightText: Copyright 2023 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "core/file_sys/fsa/fs_i_directory.h"
#include "core/hle/service/cmif_types.h"
#include "core/hle/service/filesystem/filesystem.h"
#include "core/hle/service/service.h"

namespace FileSys {
struct DirectoryEntry;
}

namespace Service::FileSystem {

class IDirectory final : public ServiceFramework<IDirectory> {
public:

private:
    Result Read(Out<s64> out_count,
                const OutArray<FileSys::DirectoryEntry, BufferAttr_HipcMapAlias> out_entries);
    Result GetEntryCount(Out<s64> out_count);
};

} // namespace Service::FileSystem
