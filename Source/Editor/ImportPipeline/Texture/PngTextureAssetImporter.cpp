#include "PngTextureAssetImporter.h"

#include <Editor/ImportPipeline/Texture/TextureAssetImportSettings.h>
#include <Runtime/Containers/StringHelper.h>
#include <Runtime/Log/Terminal.h>

#include <wuffs-v0.4.c>

#include <algorithm>
#include <cmath>
#include <cstring>

namespace Horizon::Editor
{
	namespace
	{
		constexpr u32 kChannels = 4;

		class RgbaDecodeCallbacks final : public wuffs_aux::DecodeImageCallbacks
		{
		public:
			wuffs_base__pixel_format SelectPixfmt(const wuffs_base__image_config& imageConfig) final
			{
				return wuffs_base__make_pixel_format(WUFFS_BASE__PIXEL_FORMAT__RGBA_NONPREMUL);
			}
		};

		f32 SrgbToLinear(u8 value)
		{
			const f32 c = value / 255.0f;
			if (c <= 0.04045f)
				return c / 12.92f;

			return std::pow((c + 0.055f) / 1.055f, 2.4f);
		}

		u8 LinearToSrgb(f32 value)
		{
			const f32 c = value <= 0.0031308f ? value * 12.92f : 1.055f * std::pow(value, 1.0f / 2.4f) - 0.055f;
			return static_cast<u8>(std::clamp(c, 0.0f, 1.0f) * 255.0f + 0.5f);
		}

		List<u8> Downsample(const List<u8>& src, u32 srcWidth, u32 srcHeight, u32 dstWidth, u32 dstHeight, b8 isSrgb)
		{
			List<u8> dst(static_cast<usize>(dstWidth) * dstHeight * kChannels);

			for (u32 y = 0; y < dstHeight; ++y)
			{
				for (u32 x = 0; x < dstWidth; ++x)
				{
					const u32 x0 = std::min(x * 2, srcWidth - 1);
					const u32 x1 = std::min(x * 2 + 1, srcWidth - 1);
					const u32 y0 = std::min(y * 2, srcHeight - 1);
					const u32 y1 = std::min(y * 2 + 1, srcHeight - 1);

					const u8* p00 = &src[(static_cast<usize>(y0) * srcWidth + x0) * kChannels];
					const u8* p01 = &src[(static_cast<usize>(y0) * srcWidth + x1) * kChannels];
					const u8* p10 = &src[(static_cast<usize>(y1) * srcWidth + x0) * kChannels];
					const u8* p11 = &src[(static_cast<usize>(y1) * srcWidth + x1) * kChannels];
					u8* pDst = &dst[(static_cast<usize>(y) * dstWidth + x) * kChannels];

					for (u32 c = 0; c < kChannels; ++c)
					{
						if (isSrgb && c < 3)
						{
							const f32 sum = SrgbToLinear(p00[c]) + SrgbToLinear(p01[c]) + SrgbToLinear(p10[c]) + SrgbToLinear(p11[c]);
							pDst[c] = LinearToSrgb(sum * 0.25f);
						}
						else
						{
							pDst[c] = static_cast<u8>((p00[c] + p01[c] + p10[c] + p11[c] + 2) / 4);
						}
					}
				}
			}

			return dst;
		}
	}

	AssetImportResult PngTextureAssetImporter::Parse(const ImportPipelineContext& ctx, ReadOnlyList<u8> fileBytes)
	{
		if (fileBytes.GetCount() == 0)
			return AssetImportResult::EmptyInput;

		RgbaDecodeCallbacks callbacks;
		wuffs_aux::sync_io::MemoryInput input(fileBytes.GetData(), fileBytes.GetCount());
		wuffs_aux::DecodeImageResult result = wuffs_aux::DecodeImage(callbacks, input);

		if (!result.error_message.empty())
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} could not be decoded: {}", ctx.sourcePath, result.error_message);
			return AssetImportResult::CorruptedData;
		}

		const u32 width = result.pixbuf.pixcfg.width();
		const u32 height = result.pixbuf.pixcfg.height();

		if (width == 0 || height == 0)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} has no pixels", ctx.sourcePath);
			return AssetImportResult::NoImportableData;
		}

		const wuffs_base__table_u8 plane = result.pixbuf.plane(0);
		const usize rowSize = static_cast<usize>(width) * kChannels;

		m_sourcePixels.Resize(rowSize * height);
		for (u32 y = 0; y < height; ++y)
			std::memcpy(m_sourcePixels.GetData() + y * rowSize, plane.ptr + y * plane.stride, rowSize);

		m_sourceWidth = width;
		m_sourceHeight = height;
		return AssetImportResult::Success;
	}

	void PngTextureAssetImporter::Release()
	{
		m_sourcePixels.Clear();
		m_sourceWidth = 0;
		m_sourceHeight = 0;

		m_properties = {};
		m_mips.Clear();
		m_payload.Clear();
		m_hasBuilt = false;
	}

	AssetImportResult PngTextureAssetImporter::GenerateProperties(const ImportPipelineContext& ctx, List<u8>& outBytes)
	{
		const AssetImportResult result = BuildTexture(ctx);
		if (result != AssetImportResult::Success)
			return result;

		outBytes.Resize(sizeof(Engine::TextureAssetProperties));
		std::memcpy(outBytes.GetData(), &m_properties, sizeof(Engine::TextureAssetProperties));
		return AssetImportResult::Success;
	}

	AssetImportResult PngTextureAssetImporter::GenerateTable(const ImportPipelineContext& ctx, List<u8>& outBytes)
	{
		const AssetImportResult result = BuildTexture(ctx);
		if (result != AssetImportResult::Success)
			return result;

		const usize tableSize = m_mips.GetCount() * sizeof(Engine::TextureAssetMip);
		outBytes.Resize(tableSize);
		std::memcpy(outBytes.GetData(), m_mips.GetData(), tableSize);
		return AssetImportResult::Success;
	}

	AssetImportResult PngTextureAssetImporter::GenerateBinary(const ImportPipelineContext& ctx, List<u8>& outBytes)
	{
		const AssetImportResult result = BuildTexture(ctx);
		if (result != AssetImportResult::Success)
			return result;

		outBytes.Resize(m_payload.GetCount());
		std::memcpy(outBytes.GetData(), m_payload.GetData(), m_payload.GetCount());
		return AssetImportResult::Success;
	}

	AssetImportResult PngTextureAssetImporter::BuildTexture(const ImportPipelineContext& ctx)
	{
		if (m_hasBuilt)
			return AssetImportResult::Success;

		const TextureAssetImportSettings defaults;
		const auto* pSettings = static_cast<const TextureAssetImportSettings*>(ctx.pSettings);
		const TextureAssetImportSettings& settings = pSettings ? *pSettings : defaults;

		if (settings.m_usage == Engine::TextureAssetUsage::Hdr)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} is an LDR image, Hdr usage cannot be cooked from it", StringHelper::FindFileName(ctx.sourcePath));
			return AssetImportResult::UnsupportedFeature;
		}

		if (settings.m_compression != TextureAssetCompression::None)
			Terminal::Warn(StringHelper::GetTypeName(this), "{} requested compression but there is no BC encoder, cooking as RGBA8", StringHelper::FindFileName(ctx.sourcePath));

		const b8 isSrgb = settings.m_usage == Engine::TextureAssetUsage::Color || settings.m_usage == Engine::TextureAssetUsage::UI;
		const b8 hasMips = settings.m_generateMips && settings.m_usage != Engine::TextureAssetUsage::UI;

		List<u8> level = m_sourcePixels;
		u32 width = m_sourceWidth;
		u32 height = m_sourceHeight;

		if (settings.m_usage == Engine::TextureAssetUsage::Normal && settings.m_flipGreenChannel)
		{
			for (usize i = 1; i < level.GetCount(); i += kChannels)
				level[i] = 255 - level[i];
		}

		while (settings.m_maxSize != 0 && (width > settings.m_maxSize || height > settings.m_maxSize))
		{
			const u32 nextWidth = std::max(1u, width / 2);
			const u32 nextHeight = std::max(1u, height / 2);

			level = Downsample(level, width, height, nextWidth, nextHeight, isSrgb);
			width = nextWidth;
			height = nextHeight;
		}

		m_mips.Clear();
		m_payload.Clear();

		while (true)
		{
			Engine::TextureAssetMip mip = {};
			mip.offset = m_payload.GetCount();
			mip.size = level.GetCount();
			mip.width = width;
			mip.height = height;
			mip.rowPitch = width * kChannels;
			m_mips.PushBack(mip);

			const usize writeOffset = m_payload.GetCount();
			m_payload.Resize(writeOffset + level.GetCount());
			std::memcpy(m_payload.GetData() + writeOffset, level.GetData(), level.GetCount());

			if (!hasMips || (width == 1 && height == 1))
				break;

			const u32 nextWidth = std::max(1u, width / 2);
			const u32 nextHeight = std::max(1u, height / 2);

			level = Downsample(level, width, height, nextWidth, nextHeight, isSrgb);
			width = nextWidth;
			height = nextHeight;
		}

		m_properties = {};
		m_properties.format = isSrgb ? RHI::TextureFormat::RGBA8_SRGB : RHI::TextureFormat::RGBA8_UNORM;
		m_properties.usage = settings.m_usage;
		m_properties.width = m_mips[0].width;
		m_properties.height = m_mips[0].height;
		m_properties.mipCount = static_cast<u32>(m_mips.GetCount());

		m_hasBuilt = true;
		return AssetImportResult::Success;
	}
}