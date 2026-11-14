#include "DefaultInputActionMapAssetImporter.h"

#include <Engine/Asset/Input/InputActionMapSerializer.h>
#include <Runtime/Containers/StringHelper.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/Serialization/BinaryArchive.h>
#include <Runtime/Serialization/JsonArchive.h>

namespace Horizon::Editor
{
	DefaultInputActionMapAssetImporter::~DefaultInputActionMapAssetImporter()
	{
		Release();
	}

	AssetImportResult DefaultInputActionMapAssetImporter::Parse(const ImportPipelineContext& ctx, ReadOnlyList<u8> fileBytes)
	{
		if (fileBytes.GetCount() == 0)
			return AssetImportResult::EmptyInput;

		Release();

		const std::string_view text(reinterpret_cast<const c8*>(fileBytes.GetData()), fileBytes.GetCount());
		JsonArchiveReader reader(text);

		if (!Asset::InputActionMapSerializer::Deserialize(m_actions, reader) || reader.HasError())
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} is not a valid input action map archive", ctx.sourcePath);
			Release();
			return AssetImportResult::CorruptedData;
		}

		m_parsed = true;
		return AssetImportResult::Success;
	}

	void DefaultInputActionMapAssetImporter::Release()
	{
		m_actions.Clear();
		m_parsed = false;
	}

	AssetImportResult DefaultInputActionMapAssetImporter::GenerateProperties(const ImportPipelineContext& ctx, List<u8>& outBytes)
	{
		outBytes.Clear();
		return AssetImportResult::Success;
	}

	AssetImportResult DefaultInputActionMapAssetImporter::GenerateTable(const ImportPipelineContext& ctx, List<u8>& outBytes)
	{
		outBytes.Clear();
		return AssetImportResult::Success;
	}

	AssetImportResult DefaultInputActionMapAssetImporter::GenerateBinary(const ImportPipelineContext& ctx, List<u8>& outBytes)
	{
		if (!m_parsed)
			return AssetImportResult::InternalError;

		BinaryArchiveWriter writer;

		if (!Asset::InputActionMapSerializer::Serialize(m_actions, writer) || !writer.IsComplete())
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} input action map could not be serialized to binary", ctx.sourcePath);
			return AssetImportResult::SerializationFailed;
		}

		outBytes = writer.GetBytes();
		return AssetImportResult::Success;
	}
}
