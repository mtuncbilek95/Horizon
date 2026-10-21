#include "DefaultTextureAssetImporter.h"

#include <Editor/ImportPipeline/Texture/TextureAssetImportSettings.h>
#include <Runtime/Containers/StringOps.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/Math/Vec2u.h>
#include <Runtime/RHI/Texture/GfxTextureFormat.h>

#include <wuffs-v0.4.c>

namespace Horizon::Editor
{
	namespace
	{
		constexpr u32 BytesPerPixel = 4;

		class RgbaDecodeCallbacks final : public wuffs_aux::DecodeImageCallbacks
		{
		public:
			wuffs_base__pixel_format SelectPixfmt(const wuffs_base__image_config& imageConfig) final
			{
				return wuffs_base__make_pixel_format(WUFFS_BASE__PIXEL_FORMAT__RGBA_NONPREMUL);
			}
		};
	}

	AssetImportResult DefaultTextureAssetImporter::Parse(const ImportPipelineContext& ctx, ReadOnlyList<u8> fileBytes)
	{
		if (fileBytes.GetCount() == 0)
			return AssetImportResult::EmptyInput;

		RgbaDecodeCallbacks callbacks;
		wuffs_aux::sync_io::MemoryInput input(fileBytes.GetData(), fileBytes.GetCount());
		wuffs_aux::DecodeImageResult decoded = wuffs_aux::DecodeImage(callbacks, input);

		if (!decoded.error_message.empty())
		{
			Terminal::Error(StringOps::GetName(this), "{} could not be decoded: {}", ctx.sourcePath, decoded.error_message);
			return AssetImportResult::CorruptedData;
		}

		const u32 width = decoded.pixbuf.pixcfg.width();
		const u32 height = decoded.pixbuf.pixcfg.height();
		const wuffs_base__table_u8 plane = decoded.pixbuf.plane(0);

		if (width == 0 || height == 0 || plane.ptr == nullptr)
		{
			Terminal::Error(StringOps::GetName(this), "{} has no pixels to import", ctx.sourcePath);
			return AssetImportResult::NoImportableData;
		}

		const auto* pSettings = static_cast<const TextureAssetImportSettings*>(ctx.pSettings);
		const Engine::TextureAssetUsage usage = pSettings ? pSettings->m_usage : Engine::TextureAssetUsage::Color;

		if (pSettings && pSettings->m_compressed)
			Terminal::Warn(StringOps::GetName(this), "{} asks for compression, BC encoding is not supported yet. Importing uncompressed", ctx.sourcePath);

		const usize rowSize = (usize)width * BytesPerPixel;
		m_payload.Resize(rowSize * height);

		for (u32 row = 0; row < height; ++row)
			std::memcpy(m_payload.GetData() + row * rowSize, plane.ptr + (usize)row * plane.stride, rowSize);

		BuildMipChain(width, height);

		m_properties = {};
		m_properties.format = usage == Engine::TextureAssetUsage::Color ? RHI::GfxTextureFormat::RGBA8_SRGB : RHI::GfxTextureFormat::RGBA8_UNORM;
		m_properties.assetUsage = usage;
		m_properties.textureSize = Math::Vec2u(width, height);
		m_properties.mipCount = (u32)m_mips.GetCount();
		m_properties.textureByteSize = m_payload.GetCount();
		return AssetImportResult::Success;
	}

	void DefaultTextureAssetImporter::Release()
	{
		m_payload.Clear();
		m_mips.Clear();
		m_properties = {};
	}

	usize DefaultTextureAssetImporter::GetProductCount() const
	{
		return m_payload.IsEmpty() ? 0 : 1;
	}

	ImportDeclaredProduct DefaultTextureAssetImporter::GetProduct(usize index) const
	{
		ImportDeclaredProduct product;
		product.assetType = Reflect::TypeOf<Engine::TextureAsset>();
		return product;
	}

	void DefaultTextureAssetImporter::GatherDependencies(const ImportPipelineContext& ctx, List<ImportDependency>& outDependencies)
	{
	}

	AssetImportResult DefaultTextureAssetImporter::GenerateProperties(const ImportPipelineContext& ctx, usize product, List<u8>& outBytes)
	{
		if (m_payload.IsEmpty())
			return AssetImportResult::InternalError;

		outBytes.Resize(sizeof(Engine::TextureAssetProperties));
		std::memcpy(outBytes.GetData(), &m_properties, sizeof(Engine::TextureAssetProperties));
		return AssetImportResult::Success;
	}

	AssetImportResult DefaultTextureAssetImporter::GenerateTable(const ImportPipelineContext& ctx, usize product, List<u8>& outBytes)
	{
		if (m_mips.IsEmpty())
			return AssetImportResult::InternalError;

		const usize tableSize = m_mips.GetCount() * sizeof(Engine::TextureAssetMip);
		outBytes.Resize(tableSize);
		std::memcpy(outBytes.GetData(), m_mips.GetData(), tableSize);
		return AssetImportResult::Success;
	}

	AssetImportResult DefaultTextureAssetImporter::GenerateBinary(const ImportPipelineContext& ctx, usize product, List<u8>& outBytes)
	{
		if (m_payload.IsEmpty())
			return AssetImportResult::InternalError;

		outBytes.Resize(m_payload.GetCount());
		std::memcpy(outBytes.GetData(), m_payload.GetData(), m_payload.GetCount());
		return AssetImportResult::Success;
	}

	void DefaultTextureAssetImporter::BuildMipChain(u32 width, u32 height)
	{
		m_mips.Clear();

		Engine::TextureAssetMip topMip = {};
		topMip.offset = 0;
		topMip.byteSize = m_payload.GetCount();
		topMip.width = width;
		topMip.height = height;
		m_mips.PushBack(topMip);

		while (m_mips.Back().width > 1 || m_mips.Back().height > 1)
		{
			const Engine::TextureAssetMip source = m_mips.Back();

			Engine::TextureAssetMip mip = {};
			mip.width = source.width > 1 ? source.width / 2 : 1;
			mip.height = source.height > 1 ? source.height / 2 : 1;
			mip.offset = m_payload.GetCount();
			mip.byteSize = (u64)mip.width * mip.height * BytesPerPixel;

			m_payload.Resize(mip.offset + mip.byteSize);

			const u8* pSource = m_payload.GetData() + source.offset;
			u8* pTarget = m_payload.GetData() + mip.offset;

			for (u32 y = 0; y < mip.height; ++y)
			{
				const u32 y0 = y * 2;
				const u32 y1 = y0 + 1 < source.height ? y0 + 1 : y0;

				for (u32 x = 0; x < mip.width; ++x)
				{
					const u32 x0 = x * 2;
					const u32 x1 = x0 + 1 < source.width ? x0 + 1 : x0;

					const u8* pTopLeft = pSource + ((usize)y0 * source.width + x0) * BytesPerPixel;
					const u8* pTopRight = pSource + ((usize)y0 * source.width + x1) * BytesPerPixel;
					const u8* pBottomLeft = pSource + ((usize)y1 * source.width + x0) * BytesPerPixel;
					const u8* pBottomRight = pSource + ((usize)y1 * source.width + x1) * BytesPerPixel;

					u8* pOut = pTarget + ((usize)y * mip.width + x) * BytesPerPixel;

					for (u32 channel = 0; channel < BytesPerPixel; ++channel)
						pOut[channel] = (u8)((pTopLeft[channel] + pTopRight[channel] + pBottomLeft[channel] + pBottomRight[channel] + 2) / 4);
				}
			}

			m_mips.PushBack(mip);
		}
	}
}