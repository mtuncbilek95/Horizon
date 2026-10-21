#include "DefaultShaderAssetImporter.h"

#include <Editor/ImportPipeline/Shader/ShaderAssetImportSettings.h>
#include <Runtime/Containers/StringOps.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/RHI/Shader/GfxShaderCompiler.h>

namespace Horizon::Editor
{
	AssetImportResult DefaultShaderAssetImporter::Parse(const ImportPipelineContext& ctx, ReadOnlyList<u8> fileBytes)
	{
		if (fileBytes.GetCount() == 0)
			return AssetImportResult::EmptyInput;

		List<u8> vertexCode = RHI::GfxShaderCompiler::Compile(ctx.sourcePath, RHI::GfxShaderStage::Vertex, "VSMain", ctx.sourceFolder);
		if (vertexCode.IsEmpty())
		{
			Terminal::Error(StringOps::GetName(this), "{} could not be compiled with entry point {}", ctx.sourcePath, "VSMain");
			return AssetImportResult::InvalidData;
		}

		List<u8> pixelCode = RHI::GfxShaderCompiler::Compile(ctx.sourcePath, RHI::GfxShaderStage::Pixel, "PSMain", ctx.sourceFolder);
		if (pixelCode.IsEmpty())
		{
			Terminal::Error(StringOps::GetName(this), "{} could not be compiled with entry point {}", ctx.sourcePath, "PSMain");
			return AssetImportResult::InvalidData;
		}

		Engine::ShaderAssetStage vertexStage = {};
		vertexStage.stage = RHI::GfxShaderStage::Vertex;
		vertexStage.offset = 0;
		vertexStage.byteSize = vertexCode.GetCount();

		Engine::ShaderAssetStage pixelStage = {};
		pixelStage.stage = RHI::GfxShaderStage::Pixel;
		pixelStage.offset = vertexCode.GetCount();
		pixelStage.byteSize = pixelCode.GetCount();

		m_payload.Resize(vertexCode.GetCount() + pixelCode.GetCount());
		std::memcpy(m_payload.GetData(), vertexCode.GetData(), vertexCode.GetCount());
		std::memcpy(m_payload.GetData() + pixelStage.offset, pixelCode.GetData(), pixelCode.GetCount());

		m_stages.Clear();
		m_stages.PushBack(vertexStage);
		m_stages.PushBack(pixelStage);

		const auto* pSettings = static_cast<const ShaderAssetImportSettings*>(ctx.pSettings);

		m_properties = {};
		m_properties.renderState.blendMode = pSettings ? pSettings->m_blendMode : Engine::MaterialBlendMode::Opaque;
		m_properties.renderState.twoSided = pSettings && pSettings->m_twoSided;
		m_properties.stageCount = (u32)m_stages.GetCount();
		return AssetImportResult::Success;
	}

	void DefaultShaderAssetImporter::Release()
	{
		m_payload.Clear();
		m_stages.Clear();
		m_properties = {};
	}

	usize DefaultShaderAssetImporter::GetProductCount() const
	{
		return m_payload.IsEmpty() ? 0 : 1;
	}

	ImportDeclaredProduct DefaultShaderAssetImporter::GetProduct(usize index) const
	{
		ImportDeclaredProduct product;
		product.assetType = Reflect::TypeOf<Engine::ShaderAsset>();
		return product;
	}

	void DefaultShaderAssetImporter::GatherDependencies(const ImportPipelineContext& ctx, List<ImportDependency>& outDependencies)
	{
	}

	AssetImportResult DefaultShaderAssetImporter::GenerateProperties(const ImportPipelineContext& ctx, usize product, List<u8>& outBytes)
	{
		if (m_payload.IsEmpty())
			return AssetImportResult::InternalError;

		outBytes.Resize(sizeof(Engine::ShaderAssetProperties));
		std::memcpy(outBytes.GetData(), &m_properties, sizeof(Engine::ShaderAssetProperties));
		return AssetImportResult::Success;
	}

	AssetImportResult DefaultShaderAssetImporter::GenerateTable(const ImportPipelineContext& ctx, usize product, List<u8>& outBytes)
	{
		if (m_stages.IsEmpty())
			return AssetImportResult::InternalError;

		const usize tableSize = m_stages.GetCount() * sizeof(Engine::ShaderAssetStage);
		outBytes.Resize(tableSize);
		std::memcpy(outBytes.GetData(), m_stages.GetData(), tableSize);
		return AssetImportResult::Success;
	}

	AssetImportResult DefaultShaderAssetImporter::GenerateBinary(const ImportPipelineContext& ctx, usize product, List<u8>& outBytes)
	{
		if (m_payload.IsEmpty())
			return AssetImportResult::InternalError;

		outBytes.Resize(m_payload.GetCount());
		std::memcpy(outBytes.GetData(), m_payload.GetData(), m_payload.GetCount());
		return AssetImportResult::Success;
	}
}