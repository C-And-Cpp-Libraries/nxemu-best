// SPDX-FileCopyrightText: 2021 yuzu Emulator Project
// SPDX-FileCopyrightText: 2021 Skyline Team and Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <array>
#include "yuzu_common/bit_field.h"
#include "yuzu_common/common_funcs.h"
#include "yuzu_common/common_types.h"

namespace Core {
struct Asid {
    size_t id;
};
} // namespace Core

namespace Service::Nvidia {

constexpr u32 MaxSyncPoints = 192;
constexpr u32 MaxNvEvents = 64;
using DeviceFD = s32;

constexpr DeviceFD INVALID_NVDRV_FD = -1;

enum class NvResult : u32 {
    Success = 0x0,
    NotImplemented = 0x1,
    NotSupported = 0x2,
    NotInitialized = 0x3,
    BadParameter = 0x4,
    Timeout = 0x5,
    InsufficientMemory = 0x6,
    ReadOnlyAttribute = 0x7,
    InvalidState = 0x8,
    InvalidAddress = 0x9,
    InvalidSize = 0xA,
    BadValue = 0xB,
    AlreadyAllocated = 0xD,
    Busy = 0xE,
    ResourceError = 0xF,
    CountMismatch = 0x10,
    OverFlow = 0x11,
    InsufficientTransferMemory = 0x1000,
    InsufficientVideoMemory = 0x10000,
    BadSurfaceColorScheme = 0x10001,
    InvalidSurface = 0x10002,
    SurfaceNotSupported = 0x10003,
    DispInitFailed = 0x20000,
    DispAlreadyAttached = 0x20001,
    DispTooManyDisplays = 0x20002,
    DispNoDisplaysAttached = 0x20003,
    DispModeNotSupported = 0x20004,
    DispNotFound = 0x20005,
    DispAttachDisallowed = 0x20006,
    DispTypeNotSupported = 0x20007,
    DispAuthenticationFailed = 0x20008,
    DispNotAttached = 0x20009,
    DispSamePwrState = 0x2000A,
    DispEdidFailure = 0x2000B,
    DispDsiReadAckError = 0x2000C,
    DispDsiReadInvalidResp = 0x2000D,
    FileWriteFailed = 0x30000,
    FileReadFailed = 0x30001,
    EndOfFile = 0x30002,
    FileOperationFailed = 0x30003,
    DirOperationFailed = 0x30004,
    EndOfDirList = 0x30005,
    ConfigVarNotFound = 0x30006,
    InvalidConfigVar = 0x30007,
    LibraryNotFound = 0x30008,
    SymbolNotFound = 0x30009,
    MemoryMapFailed = 0x3000A,
    IoctlFailed = 0x3000F,
    AccessDenied = 0x30010,
    DeviceNotFound = 0x30011,
    KernelDriverNotFound = 0x30012,
    FileNotFound = 0x30013,
    PathAlreadyExists = 0x30014,
    ModuleNotPresent = 0xA000E,
};

// obtained from
// https://github.com/skyline-emu/skyline/blob/nvdec-dev/app/src/main/cpp/skyline/services/nvdrv/devices/nvhost/ctrl.h#L47
enum class EventState {
    Available = 0,
    Waiting = 1,
    Cancelling = 2,
    Signalling = 3,
    Signalled = 4,
    Cancelled = 5,
};

union Ioctl {
    u32_le raw;
    BitField<0, 8, u32> cmd;
    BitField<8, 8, u32> group;
    BitField<16, 14, u32> length;
    BitField<30, 1, u32> is_in;
    BitField<31, 1, u32> is_out;
};

} // namespace Service::Nvidia

namespace Service::android {

enum class BufferTransformFlags : u32 {
    /// No transform flags are set
    Unset = 0x00,
    /// Flip source image horizontally (around the vertical axis)
    FlipH = 0x01,
    /// Flip source image vertically (around the horizontal axis)
    FlipV = 0x02,
    /// Rotate source image 90 degrees clockwise
    Rotate90 = 0x04,
    /// Rotate source image 180 degrees
    Rotate180 = 0x03,
    /// Rotate source image 270 degrees clockwise
    Rotate270 = 0x07,
};
DECLARE_ENUM_FLAG_OPERATORS(BufferTransformFlags);

enum class PixelFormat : u32 {
    NoFormat = 0,
    Rgba8888 = 1,
    Rgbx8888 = 2,
    Rgb888 = 3,
    Rgb565 = 4,
    Bgra8888 = 5,
    Rgba5551 = 6,
    Rgba4444 = 7,
};

} // namespace Service::android

namespace Tegra {

enum class BlendMode {
    Opaque,
    Premultiplied,
    Coverage,
};

} // namespace Tegra
