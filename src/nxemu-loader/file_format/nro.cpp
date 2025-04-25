#include "nro.h"
#include "nacp.h"
#include <array>
#include <common/file.h>
#include <common/path.h>

typedef struct
{
    uint8_t Magic[4];
    uint32_t FormatVersion;
    uint64_t IconOffset;
    uint64_t IconSize;
    uint64_t NacpOffset;
    uint64_t NacpSize;
    uint64_t RomFSOffset;
    uint64_t RomFSSize;
} NRO_ASSET_HEADER;
static_assert(sizeof(NRO_ASSET_HEADER) == 0x38, "NRO_ASSET_HEADER has incorrect size.");

bool Nro::IsNroFile(const char * file)
{
    Path filePath(file);
    if (!filePath.FileExists() || _stricmp(filePath.GetExtension().c_str(), "nro") != 0)
    {
        return false;
    }

    File readFile(filePath, IFile::modeRead);
    if (!readFile.IsOpen())
    {
        return false;
    }

    NRO_HEADER header;
    readFile.SeekToBegin();
    if (!readFile.Read(&header, sizeof(header)))
    {
        return false;
    }
    if (*((uint32_t *)(&header.Signature[0])) != *((uint32_t *)(&"NRO0")))
    {
        return false;
    }
    return true;
}

Nro::Nro(const char * file) :
    m_header({0}),
    m_moduleHeader({0}),
    m_nacp(nullptr),
    m_bssSize(0),
    m_valid(false)
{
    Path filePath(file);
    if (!filePath.FileExists() || _stricmp(filePath.GetExtension().c_str(), "nro") != 0)
    {
        return;
    }

    File readFile(filePath, IFile::modeRead);
    if (!readFile.IsOpen())
    {
        return;
    }

    readFile.SeekToBegin();
    if (!readFile.Read(&m_header, sizeof(m_header)))
    {
        return;
    }
    if (*((uint32_t *)(&m_header.Signature[0])) != *((uint32_t *)(&"NRO0")))
    {
        return;
    }

    if (readFile.GetLength() >= m_header.Size + sizeof(NRO_ASSET_HEADER))
    {
        NRO_ASSET_HEADER assetHeader;
        readFile.Seek(m_header.Size, IFile::SeekPosition::begin);
        if (readFile.Read(&assetHeader, sizeof(NRO_ASSET_HEADER)) != sizeof(NRO_ASSET_HEADER))
        {
            return;
        }

        if (assetHeader.FormatVersion != 0)
        {
            return;
        }

        if (*((uint32_t *)(&assetHeader.Magic[0])) != *((uint32_t *)(&"ASET")))
        {
            return;
        }

        if (assetHeader.NacpSize > 0)
        {
            m_nacp = std::make_unique<NACP>();
            if (!m_nacp->Load(readFile, assetHeader.NacpOffset + m_header.Size, 0, assetHeader.NacpSize))
            {
                return;
            }
        }
    }

    if (m_header.ModuleOffset + sizeof(NRO_MODULE_HEADER) > readFile.GetLength())
    {
        return;
    }
    readFile.Seek(m_header.ModuleOffset, IFile::SeekPosition::begin);
    readFile.Read(&m_moduleHeader, sizeof(m_moduleHeader));
    m_bssSize = HasModHeader() ? PageAlignSize(m_moduleHeader.BssEndOffset - m_moduleHeader.BssStartOffset) : PageAlignSize(m_header.BssSize);

    m_programImage.resize(PageAlignSize(m_header.Size) + m_bssSize);
    readFile.SeekToBegin();
    if (!readFile.Read(m_programImage.data(), (uint32_t)m_programImage.size()))
    {
        return;
    }
    m_valid = true;
}

NACP * Nro::Nacp() const
{
    return m_nacp.get();
}

const uint8_t * Nro::Data(void) const
{
    return m_programImage.data();
}

uint32_t Nro::DataSize(void) const
{
    return (uint32_t)m_programImage.size();
}

uint64_t Nro::CodeSegmentAddr(void) const
{
    return m_header.TextMemoryOffset;
}

uint64_t Nro::CodeSegmentOffset(void) const
{
    return m_header.TextMemoryOffset;
}

uint64_t Nro::CodeSegmentSize(void) const
{
    return PageAlignSize(m_header.TextSize);
}

uint64_t Nro::RODataSegmentAddr(void) const
{
    return m_header.RoMemoryOffset;
}

uint64_t Nro::RODataSegmentOffset(void) const
{
    return m_header.RoMemoryOffset;
}

uint64_t Nro::RODataSegmentSize(void) const
{
    return PageAlignSize(m_header.RoSize);
}

uint64_t Nro::DataSegmentAddr(void) const
{
    return m_header.DataMemoryOffset;
}

uint64_t Nro::DataSegmentOffset(void) const
{
    return m_header.DataMemoryOffset;
}

uint64_t Nro::DataSegmentSize(void) const
{
    return PageAlignSize(m_header.DataSize) + m_bssSize;
}

uint32_t Nro::CodeSize()
{
    return PageAlignSize(m_header.Size) + m_bssSize;
}

const IProgramMetadata & Nro::MetaData() const
{
    class NroMetaData :
        public IProgramMetadata
    {
        bool Is64BitProgram() const
        {
            return true;
        }
        ProgramAddressSpaceType GetAddressSpaceType() const
        {
            return ProgramAddressSpaceType::Is39Bit;
        }
        uint8_t GetMainThreadPriority() const
        {
            return 0x2C;
        }
        uint8_t GetMainThreadCore() const
        {
            return 0;
        }
        uint32_t GetMainThreadStackSize() const
        {
            return 0x100000;
        }
        uint64_t GetTitleID() const
        {
            return 0;
        }
        uint64_t GetFilesystemPermissions() const
        {
            return 0xFFFFFFFFFFFFFFFF;
        }
        uint32_t GetSystemResourceSize() const
        {
            return 0;
        }
        PoolPartition GetPoolPartition() const
        {
            return PoolPartition::Application;
        }
        const uint32_t * GetKernelCapabilities() const
        {
            return &kernelCapabilities[0];
        }
        uint32_t GetKernelCapabilitiesSize() const
        {
            return (uint32_t)kernelCapabilities.size();
        }
        const char * GetName() const
        {
            return "";
        }

        std::array<uint32_t, 1> kernelCapabilities = {0x30043F7};
    };
    static NroMetaData defaultMetaData;
    return defaultMetaData;
}

bool Nro::Valid() const
{
    return m_valid;
}

bool Nro::HasModHeader() const
{
    return (*((uint32_t *)(&m_moduleHeader.Signature[0])) == *((uint32_t *)(&"MOD0")));
}

constexpr uint32_t Nro::PageAlignSize(uint32_t size)
{
    enum
    {
        pageBits = 12,
        pageSize = 1ULL << pageBits,
        pageMask = pageSize - 1,
    };
    return ((size + pageMask) & ~pageMask);
}
