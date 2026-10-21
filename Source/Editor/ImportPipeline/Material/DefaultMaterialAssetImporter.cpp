#include "DefaultMaterialAssetImporter.h"

#include <Engine/Reflection/ReflectionSystem.h>
#include <Runtime/Containers/StringOps.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/Serialization/JsonArchive.h>
#include <Runtime/Serialization/Serializer.h>

#include <cstring>
#include <string_view>

namespace Horizon::Editor
{
	namespace
	{
		const Reflect::Type* ResolveType(void* pUserData, Reflect::TypeHandle handle)
		{
			return static_cast<Engine::ReflectionSystem*>(pUserData)->GetType(handle);
		}

		const Reflect::Type* ResolveTypeByName(void* pUserData, const std::string& name)
		{
			return static_cast<Engine::ReflectionSystem*>(pUserData)->GetTypeByName(name);
		}
	}

	AssetImportResult DefaultMaterialAssetImporter::Parse(const ImportPipelineContext& ctx, ReadOnlyList<u8> fileBytes)
	{
		if (fileBytes.GetCount() == 0)
			return AssetImportResult::EmptyInput;

		if (!ctx.pReflection)
		{
			Terminal::Error(StringOps::GetName(this), "{} cannot be parsed without a reflection system", ctx.sourcePath);
			return AssetImportResult::InternalError;
		}

		const Reflect::Type* pDocumentType = ctx.pReflection->GetType(Reflect::TypeOf<MaterialDocument>());
		if (!pDocumentType)
		{
			Terminal::Error(StringOps::GetName(this), "MaterialDocument is not registered to the reflection system");
			return AssetImportResult::InternalError;
		}

		Release();

		const std::string_view text(reinterpret_cast<const c8*>(fileBytes.GetData()), fileBytes.GetCount());
		JsonArchiveReader reader(text);

		Serializer serializer(ctx.pReflection, ResolveType, ResolveTypeByName);
		serializer.Deserialize(&m_document, *pDocumentType, reader);

		if (reader.HasError())
		{
			Terminal::Error(StringOps::GetName(this), "{} is not a valid material document", ctx.sourcePath);
			Release();
			return AssetImportResult::CorruptedData;
		}

		for (usize i = 0; i < m_document.m_params.GetCount(); ++i)
		{
			const MaterialParamEntry& entry = m_document.m_params[i];

			if (entry.m_name.empty())
			{
				Terminal::Error(StringOps::GetName(this), "{} has a parameter without a name at index {}", ctx.sourcePath, i);
				Release();
				return AssetImportResult::InvalidData;
			}

			const u64 nameHash = Reflect::Detail::Fnv1a(entry.m_name.c_str());

			for (const Engine::MaterialParamValue& existing : m_values)
			{
				if (existing.nameHash != nameHash)
					continue;

				Terminal::Error(StringOps::GetName(this), "{} declares {} more than once", ctx.sourcePath, entry.m_name);
				Release();
				return AssetImportResult::InvalidData;
			}

			if (entry.m_type == Engine::MaterialParamType::Texture && !entry.m_texture.GetId().IsValid())
				Terminal::Warn(StringOps::GetName(this), "{} parameter {} has no texture, the fallback texture is used", ctx.sourcePath, entry.m_name);

			const f32 components[4] = { entry.m_value.X(), entry.m_value.Y(), entry.m_value.Z(), entry.m_value.W() };

			Engine::MaterialParamValue value = {};
			value.nameHash = nameHash;
			value.type = entry.m_type;
			value.textureId = entry.m_texture.GetId();
			std::memcpy(value.value, components, sizeof(components));

			m_values.PushBack(value);
		}

		if (!m_document.m_shader.GetId().IsValid())
			Terminal::Warn(StringOps::GetName(this), "{} has no shader, it is drawn with the fallback until one is picked", ctx.sourcePath);

		m_properties = {};
		m_properties.shaderId = m_document.m_shader.GetId();
		m_properties.valueCount = (u32)m_values.GetCount();

		m_isParsed = true;
		return AssetImportResult::Success;
	}

	void DefaultMaterialAssetImporter::Release()
	{
		m_document.m_shader.SetId(Guid());
		m_document.m_params.Clear();

		m_values.Clear();
		m_properties = {};
		m_isParsed = false;
	}

	usize DefaultMaterialAssetImporter::GetProductCount() const
	{
		return m_isParsed ? 1 : 0;
	}

	ImportDeclaredProduct DefaultMaterialAssetImporter::GetProduct(usize index) const
	{
		ImportDeclaredProduct product;
		product.assetType = Reflect::TypeOf<Engine::MaterialAsset>();
		return product;
	}

	void DefaultMaterialAssetImporter::GatherDependencies(const ImportPipelineContext& ctx, List<ImportDependency>& outDependencies)
	{
	}

	AssetImportResult DefaultMaterialAssetImporter::GenerateProperties(const ImportPipelineContext& ctx, usize product, List<u8>& outBytes)
	{
		if (!m_isParsed)
			return AssetImportResult::InternalError;

		outBytes.Resize(sizeof(Engine::MaterialAssetProperties));
		std::memcpy(outBytes.GetData(), &m_properties, sizeof(Engine::MaterialAssetProperties));
		return AssetImportResult::Success;
	}

	AssetImportResult DefaultMaterialAssetImporter::GenerateTable(const ImportPipelineContext& ctx, usize product, List<u8>& outBytes)
	{
		if (!m_isParsed)
			return AssetImportResult::InternalError;

		const usize tableSize = m_values.GetCount() * sizeof(Engine::MaterialParamValue);
		outBytes.Resize(tableSize);

		if (tableSize > 0)
			std::memcpy(outBytes.GetData(), m_values.GetData(), tableSize);

		return AssetImportResult::Success;
	}

	AssetImportResult DefaultMaterialAssetImporter::GenerateBinary(const ImportPipelineContext& ctx, usize product, List<u8>& outBytes)
	{
		outBytes.Clear();
		return AssetImportResult::Success;
	}
}