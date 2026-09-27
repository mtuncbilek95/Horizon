#include "SceneAssetImporter.h"

#include <Engine/Reflection/ReflectionSystem.h>
#include <Engine/Asset/Scene/SceneSerializer.h>
#include <Engine/World/ECS/Scene.h>
#include <Runtime/Containers/StringOps.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/PAL/File/File.h>
#include <Runtime/Serialization/BinaryArchive.h>
#include <Runtime/Serialization/JsonArchive.h>


namespace Horizon::Editor
{
	AssetImportResult SceneAssetImporter::ImportAsset(Engine::Engine* pEngine, const std::string& inPath, List<u8>& outByteArr)
	{
		PAL::FileAccessRequest request = PAL::File::RequestAccess(inPath, PAL::FileOperationAccessPolicy::Read,
			PAL::FileOperationSharePolicy::SharedRead);

		std::string text;
		const b8 wasRead = PAL::File::ReadString(request, text);

		PAL::File::ReleaseAccess(request);

		if (!wasRead)
		{
			Terminal::Error(StringOps::GetName(this), "{} cannot be read", inPath);
			return AssetImportResult::EmptyInput;
		}

		JsonArchiveReader reader(text);

		if (reader.HasError())
		{
			Terminal::Error(StringOps::GetName(this), "{} is not a valid scene file", inPath);
			return AssetImportResult::InvalidFormat;
		}

		Engine::ReflectionSystem* pReflection = pEngine->GetReflectionSystem();
		Engine::Scene scene(pReflection);

		if (!Engine::SceneSerializer::Deserialize(scene, pReflection, reader))
		{
			Terminal::Error(StringOps::GetName(this), "{} could not be loaded as a scene", inPath);
			return AssetImportResult::InvalidFormat;
		}

		BinaryArchiveWriter writer;
		Engine::SceneSerializer::Serialize(scene, pReflection, writer);

		if (!writer.IsComplete())
		{
			Terminal::Error(StringOps::GetName(this), "{} could not be cooked", inPath);
			return AssetImportResult::InternalError;
		}

		Engine::SceneProperties properties = {};
		properties.entityCount = scene.GetEntityCount();

		const List<u8>& payload = writer.GetBytes();
		const usize propertyBytes = sizeof(Engine::SceneProperties);

		outByteArr.Resize(propertyBytes + payload.GetCount());

		u8* pCursor = outByteArr.GetData();

		std::memcpy(pCursor, &properties, propertyBytes);
		pCursor += propertyBytes;

		if (!payload.IsEmpty())
			std::memcpy(pCursor, payload.GetData(), payload.GetCount());

		return AssetImportResult::Success;
	}
}