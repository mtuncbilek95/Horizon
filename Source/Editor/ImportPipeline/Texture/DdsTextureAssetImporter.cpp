#include "DdsTextureAssetImporter.h"

#include <Editor/ImportPipeline/Texture/TextureAssetImportSettings.h>
#include <Runtime/Containers/StringHelper.h>
#include <Runtime/Log/Terminal.h>

#define TINYDDSLOADER_IMPLEMENTATION
#include <tinyddsloader.h>

#include <algorithm>
#include <cstring>

namespace Horizon::Editor
{
	namespace
	{
		constexpr u32 kRowPitchAlignment = 256;
		constexpr usize kMipOffsetAlignment = 512;
		constexpr u32 kMinMipSize = 16;
		constexpr u32 kUncompressedChannels = 4;

		RHI::TextureFormat ConvertFormat(tinyddsloader::DDSFile::DXGIFormat format)
		{
			using DXGIFormat = tinyddsloader::DDSFile::DXGIFormat;

			switch (format)
			{
			case DXGIFormat::R8G8B8A8_UNorm:
			case DXGIFormat::R8G8B8A8_UNorm_SRGB:
				return RHI::TextureFormat::RGBA8_UNORM;
			case DXGIFormat::BC1_UNorm:
			case DXGIFormat::BC1_UNorm_SRGB:
				return RHI::TextureFormat::BC1_UNORM;
			case DXGIFormat::BC2_UNorm:
			case DXGIFormat::BC2_UNorm_SRGB:
				return RHI::TextureFormat::BC2_UNORM;
			case DXGIFormat::BC3_UNorm:
			case DXGIFormat::BC3_UNorm_SRGB:
				return RHI::TextureFormat::BC3_UNORM;
			case DXGIFormat::BC4_UNorm:
				return RHI::TextureFormat::BC4_UNORM;
			case DXGIFormat::BC4_SNorm:
				return RHI::TextureFormat::BC4_SNORM;
			case DXGIFormat::BC5_UNorm:
				return RHI::TextureFormat::BC5_UNORM;
			case DXGIFormat::BC5_SNorm:
				return RHI::TextureFormat::BC5_SNORM;
			case DXGIFormat::BC6H_UF16:
				return RHI::TextureFormat::BC6H_UFLOAT;
			case DXGIFormat::BC6H_SF16:
				return RHI::TextureFormat::BC6H_SFLOAT;
			case DXGIFormat::BC7_UNorm:
			case DXGIFormat::BC7_UNorm_SRGB:
				return RHI::TextureFormat::BC7_UNORM;
			default:
				return RHI::TextureFormat::Undefined;
			}
		}

		RHI::TextureFormat ToSrgb(RHI::TextureFormat format)
		{
			switch (format)
			{
			case RHI::TextureFormat::RGBA8_UNORM:
				return RHI::TextureFormat::RGBA8_SRGB;
			case RHI::TextureFormat::BC1_UNORM:
				return RHI::TextureFormat::BC1_SRGB;
			case RHI::TextureFormat::BC2_UNORM:
				return RHI::TextureFormat::BC2_SRGB;
			case RHI::TextureFormat::BC3_UNORM:
				return RHI::TextureFormat::BC3_SRGB;
			case RHI::TextureFormat::BC7_UNORM:
				return RHI::TextureFormat::BC7_SRGB;
			default:
				return format;
			}
		}

		u32 GetBlockBytes(RHI::TextureFormat format)
		{
			switch (format)
			{
			case RHI::TextureFormat::BC1_UNORM:
			case RHI::TextureFormat::BC4_UNORM:
			case RHI::TextureFormat::BC4_SNORM:
				return 8;
			case RHI::TextureFormat::BC2_UNORM:
			case RHI::TextureFormat::BC3_UNORM:
			case RHI::TextureFormat::BC5_UNORM:
			case RHI::TextureFormat::BC5_SNORM:
			case RHI::TextureFormat::BC6H_UFLOAT:
			case RHI::TextureFormat::BC6H_SFLOAT:
			case RHI::TextureFormat::BC7_UNORM:
				return 16;
			default:
				return 0;
			}
		}
	}

	AssetImportResult DdsTextureAssetImporter::Parse(const ImportPipelineContext& ctx, ReadOnlyList<u8> fileBytes)
	{
		if (fileBytes.GetCount() == 0)
			return AssetImportResult::EmptyInput;

		tinyddsloader::DDSFile file;
		if (file.Load(fileBytes.GetData(), fileBytes.GetCount()) != tinyddsloader::Result::Success)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} could not be parsed", ctx.sourcePath);
			return AssetImportResult::CorruptedData;
		}

		if (file.IsCubemap() || file.GetArraySize() > 1 || file.GetTextureDimension() != tinyddsloader::DDSFile::TextureDimension::Texture2D)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} is not a single 2D texture, cubemap, array and volume sources are not supported", ctx.sourcePath);
			return AssetImportResult::UnsupportedFeature;
		}

		const RHI::TextureFormat format = ConvertFormat(file.GetFormat());
		if (format == RHI::TextureFormat::Undefined)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} has an unsupported pixel format", ctx.sourcePath);
			return AssetImportResult::UnsupportedFeature;
		}

		const u32 width = file.GetWidth();
		const u32 height = file.GetHeight();
		const u32 mipCount = file.GetMipCount();

		if (width == 0 || height == 0 || mipCount == 0)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} has no pixels", ctx.sourcePath);
			return AssetImportResult::NoImportableData;
		}

		const u32 blockBytes = GetBlockBytes(format);
		const b8 isUnaligned = blockBytes != 0 && (width % 4 != 0 || height % 4 != 0);
		const b8 isSingleBlock = isUnaligned && width <= 4 && height <= 4;

		if (isUnaligned && !isSingleBlock)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} is block compressed but its size {}x{} is not a multiple of 4", ctx.sourcePath, width, height);
			return AssetImportResult::UnsupportedFeature;
		}

		const u32 sourceMipCount = isSingleBlock ? 1 : mipCount;

		m_sourceMips.Clear();
		m_sourceMips.Resize(sourceMipCount);

		for (u32 i = 0; i < sourceMipCount; ++i)
		{
			const tinyddsloader::DDSFile::ImageData* pImage = file.GetImageData(i, 0);
			if (pImage == nullptr || pImage->m_mem == nullptr)
			{
				Terminal::Error(StringHelper::GetTypeName(this), "{} has no data for mip {}", ctx.sourcePath, i);
				return AssetImportResult::CorruptedData;
			}

			SourceMip& mip = m_sourceMips[i];
			mip.width = isSingleBlock ? 4 : pImage->m_width;
			mip.height = isSingleBlock ? 4 : pImage->m_height;

			if (blockBytes != 0)
			{
				mip.rowBytes = std::max(1u, (mip.width + 3) / 4) * blockBytes;
				mip.rowCount = std::max(1u, (mip.height + 3) / 4);
			}
			else
			{
				mip.rowBytes = mip.width * kUncompressedChannels;
				mip.rowCount = mip.height;
			}

			if (pImage->m_memPitch < mip.rowBytes)
			{
				Terminal::Error(StringHelper::GetTypeName(this), "{} has an unexpected row pitch on mip {}", ctx.sourcePath, i);
				return AssetImportResult::CorruptedData;
			}

			const u8* pSource = static_cast<const u8*>(pImage->m_mem);

			mip.bytes.Resize(static_cast<usize>(mip.rowBytes) * mip.rowCount);
			for (u32 y = 0; y < mip.rowCount; ++y)
				std::memcpy(mip.bytes.GetData() + static_cast<usize>(y) * mip.rowBytes, pSource + static_cast<usize>(y) * pImage->m_memPitch, mip.rowBytes);
		}

		m_sourceFormat = format;
		return AssetImportResult::Success;
	}

	void DdsTextureAssetImporter::Release()
	{
		m_sourceFormat = RHI::TextureFormat::Undefined;
		m_sourceMips.Clear();

		m_properties = {};
		m_mips.Clear();
		m_payload.Clear();
		m_hasBuilt = false;
	}

	AssetImportResult DdsTextureAssetImporter::GenerateProperties(const ImportPipelineContext& ctx, List<u8>& outBytes)
	{
		const AssetImportResult result = BuildTexture(ctx);
		if (result != AssetImportResult::Success)
			return result;

		outBytes.Resize(sizeof(Asset::TextureProperties));
		std::memcpy(outBytes.GetData(), &m_properties, sizeof(Asset::TextureProperties));
		return AssetImportResult::Success;
	}

	AssetImportResult DdsTextureAssetImporter::GenerateTable(const ImportPipelineContext& ctx, List<u8>& outBytes)
	{
		const AssetImportResult result = BuildTexture(ctx);
		if (result != AssetImportResult::Success)
			return result;

		const usize tableSize = m_mips.GetCount() * sizeof(Asset::TextureMip);
		outBytes.Resize(tableSize);
		std::memcpy(outBytes.GetData(), m_mips.GetData(), tableSize);
		return AssetImportResult::Success;
	}

	AssetImportResult DdsTextureAssetImporter::GenerateBinary(const ImportPipelineContext& ctx, List<u8>& outBytes)
	{
		const AssetImportResult result = BuildTexture(ctx);
		if (result != AssetImportResult::Success)
			return result;

		outBytes.Resize(m_payload.GetCount());
		std::memcpy(outBytes.GetData(), m_payload.GetData(), m_payload.GetCount());
		return AssetImportResult::Success;
	}

	AssetImportResult DdsTextureAssetImporter::BuildTexture(const ImportPipelineContext& ctx)
	{
		if (m_hasBuilt)
			return AssetImportResult::Success;

		const TextureAssetImportSettings defaults;
		const auto* pSettings = static_cast<const TextureAssetImportSettings*>(ctx.pSettings);
		const TextureAssetImportSettings& settings = pSettings ? *pSettings : defaults;

		const b8 isHdrFormat = m_sourceFormat == RHI::TextureFormat::BC6H_UFLOAT || m_sourceFormat == RHI::TextureFormat::BC6H_SFLOAT;
		const b8 isHdrUsage = settings.m_usage == Asset::TextureUsage::Hdr;

		if (isHdrFormat != isHdrUsage)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} usage does not match its format, Hdr usage requires a BC6H source and BC6H requires Hdr usage", StringHelper::FindFileName(ctx.sourcePath));
			return AssetImportResult::UnsupportedFeature;
		}

		if (settings.m_usage == Asset::TextureUsage::Normal && settings.m_flipGreenChannel)
			Terminal::Warn(StringHelper::GetTypeName(this), "{} requested a green channel flip but dds sources are cooked as is, ignoring", StringHelper::FindFileName(ctx.sourcePath));

		const b8 isSrgb = settings.m_usage == Asset::TextureUsage::Color || settings.m_usage == Asset::TextureUsage::UI;
		const b8 hasMips = settings.m_generateMips && settings.m_usage != Asset::TextureUsage::UI;

		const u32 blockBytes = GetBlockBytes(m_sourceFormat);
		const u32 sourceMipCount = static_cast<u32>(m_sourceMips.GetCount());

		u32 firstMip = 0;
		while (settings.m_maxSize != 0 && firstMip + 1 < sourceMipCount && (m_sourceMips[firstMip].width > settings.m_maxSize || m_sourceMips[firstMip].height > settings.m_maxSize))
		{
			const SourceMip& next = m_sourceMips[firstMip + 1];
			if (blockBytes != 0 && (next.width % 4 != 0 || next.height % 4 != 0))
				break;

			++firstMip;
		}

		const SourceMip& top = m_sourceMips[firstMip];
		if (hasMips && firstMip + 1 == sourceMipCount && top.width / 2 >= kMinMipSize && top.height / 2 >= kMinMipSize)
			Terminal::Warn(StringHelper::GetTypeName(this), "{} has no mip chain and mips cannot be generated for a dds source, cooking a single mip", StringHelper::FindFileName(ctx.sourcePath));

		m_mips.Clear();
		m_payload.Clear();

		for (u32 i = firstMip; i < sourceMipCount; ++i)
		{
			const SourceMip& source = m_sourceMips[i];

			if (i != firstMip && (!hasMips || source.width < kMinMipSize || source.height < kMinMipSize))
				break;

			const u32 rowPitch = (source.rowBytes + kRowPitchAlignment - 1) & ~(kRowPitchAlignment - 1);
			const usize payloadEnd = m_payload.GetCount();
			const usize mipOffset = (payloadEnd + kMipOffsetAlignment - 1) & ~(kMipOffsetAlignment - 1);
			const usize mipSize = static_cast<usize>(rowPitch) * source.rowCount;

			Asset::TextureMip mip = {};
			mip.offset = mipOffset;
			mip.size = mipSize;
			mip.width = source.width;
			mip.height = source.height;
			mip.rowPitch = rowPitch;
			m_mips.PushBack(mip);

			m_payload.Resize(mipOffset + mipSize);
			std::memset(m_payload.GetData() + payloadEnd, 0, mipOffset + mipSize - payloadEnd);

			for (u32 y = 0; y < source.rowCount; ++y)
				std::memcpy(m_payload.GetData() + mipOffset + static_cast<usize>(y) * rowPitch, source.bytes.GetData() + static_cast<usize>(y) * source.rowBytes, source.rowBytes);
		}

		m_properties = {};
		m_properties.format = isSrgb ? ToSrgb(m_sourceFormat) : m_sourceFormat;
		m_properties.usage = settings.m_usage;
		m_properties.width = m_mips[0].width;
		m_properties.height = m_mips[0].height;
		m_properties.mipCount = static_cast<u32>(m_mips.GetCount());

		m_hasBuilt = true;
		return AssetImportResult::Success;
	}
}