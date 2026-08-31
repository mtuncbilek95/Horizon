#include "SceneImporter.h"

#include <Engine/Asset/Scene/SceneChunk.h>
#include <Engine/Content/ContentFile.h>
#include <Engine/Content/ContentFileWriter.h>
#include <Engine/Reflection/ReflectionSystem.h>

#include <Runtime/Containers/StringOps.h>
#include <Runtime/Definitions/Allocator.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/PAL/File/File.h>
#include <Runtime/Serialization/BinaryArchive.h>
#include <Runtime/Serialization/JsonArchive.h>
#include <Runtime/Serialization/Serializer.h>

namespace Horizon::Editor
{
	namespace
	{
		const Reflect::Type* ResolveSceneType(void* pUserData, Reflect::TypeHandle handle)
		{
			return static_cast<Engine::ReflectionSystem*>(pUserData)->GetType(handle);
		}

		b8 ReadSource(const std::string& sourcePath, std::string& outText)
		{
			PAL::FileAccessRequest access = PAL::File::RequestAccess(sourcePath,
				PAL::FileOperationAccessPolicy::Read, PAL::FileOperationSharePolicy::SharedRead);

			const b8 wasRead = PAL::File::ReadString(access, outText);

			PAL::File::ReleaseAccess(access);

			if (!wasRead)
			{
				Terminal::Error("SceneImporter", "{} cannot be read", sourcePath);
				return false;
			}

			return true;
		}

		b8 TranscodeChunk(Engine::ReflectionSystem* pReflection, Serializer& serializer, const List<std::string>& typeNames,
			JsonArchiveReader& reader, BinaryArchiveWriter& writer)
		{
			reader.BeginObject();

			u64 typeIndex = 0;

			if (reader.Key("typeIndex"))
				typeIndex = reader.ReadU64();

			if (typeIndex >= typeNames.GetCount())
			{
				Terminal::Error("SceneImporter", "A chunk names the unknown type index {}", typeIndex);
				return false;
			}

			const std::string& typeName = typeNames[(usize)typeIndex];
			Reflect::Type* pType = pReflection->GetTypeByName(typeName);

			if (pType == nullptr)
			{
				Terminal::Error("SceneImporter", "'{}' is not registered in this build", typeName);
				return false;
			}

			List<u32> entities;

			if (reader.Key("entities"))
			{
				const usize count = reader.BeginArray();
				entities.Resize(count);

				for (usize i = 0; i < count; i++)
					entities[i] = (u32)reader.ReadU64();

				reader.EndArray();
			}

			usize objectCount = 0;
			const b8 hasObjects = reader.Key("objects");

			if (hasObjects)
				objectCount = reader.BeginArray();

			if (objectCount != entities.GetCount())
			{
				Terminal::Error("SceneImporter", "'{}' carries {} objects for {} entities", typeName, objectCount,
					entities.GetCount());
				return false;
			}

			void* pInstance = Memory::Allocator::AllocateRaw(pType->GetSizeInBytes(), pType->GetAlignment(),
				Memory::CurrLoc());

			if (pInstance == nullptr)
			{
				Terminal::Error("SceneImporter", "'{}' cannot be staged for transcoding", typeName);
				return false;
			}

			writer.BeginObject();

			writer.Key("typeIndex");
			writer.WriteU64(typeIndex);

			writer.Key("count");
			writer.WriteU64(entities.GetCount());

			writer.Key("stride");
			writer.WriteU64(pType->GetSizeInBytes());

			writer.Key("flags");
			writer.WriteU64(0);

			writer.Key("entities");
			writer.WriteBytes(entities.GetData(), entities.GetCount() * sizeof(u32));

			writer.Key("objects");
			writer.BeginArray(objectCount);

			for (usize i = 0; i < objectCount; i++)
			{
				pType->ConstructAt(pInstance);

				serializer.Deserialize(pInstance, *pType, reader);
				serializer.Serialize(pInstance, *pType, writer);

				pType->DestructAt(pInstance);
			}

			writer.EndArray();
			writer.EndObject();

			Memory::Allocator::FreeRaw(pInstance);

			if (hasObjects)
				reader.EndArray();

			reader.EndObject();

			return true;
		}
	}

	b8 SceneImporter::Import(const ImportRequest& request, ImportSink& sink)
	{
		if (request.pReflection == nullptr)
		{
			Terminal::Error(StringOps::GetName(this), "{} has no reflection system", request.sourcePath);
			return false;
		}

		std::string text;

		if (!ReadSource(request.sourcePath, text))
			return false;

		JsonArchiveReader reader(text);

		if (reader.HasError())
		{
			Terminal::Error(StringOps::GetName(this), "{} is not a valid scene document", request.sourcePath);
			return false;
		}

		BinaryArchiveWriter writer;

		reader.BeginObject();
		writer.BeginObject();

		u64 version = 0;

		if (reader.Key("version"))
			version = reader.ReadU64();

		if (version > Engine::SceneFormatVersion)
		{
			Terminal::Error(StringOps::GetName(this), "{} is version {} but only {} is supported", request.sourcePath,
				version, Engine::SceneFormatVersion);
			return false;
		}

		writer.Key("version");
		writer.WriteU64(version);

		List<std::string> typeNames;

		if (reader.Key("typeNames"))
		{
			const usize count = reader.BeginArray();
			typeNames.Resize(count);

			for (usize i = 0; i < count; i++)
				typeNames[i] = reader.ReadString();

			reader.EndArray();
		}

		writer.Key("typeNames");
		writer.BeginArray(typeNames.GetCount());

		for (const std::string& typeName : typeNames)
			writer.WriteString(typeName);

		writer.EndArray();

		writer.Key("dependencies");
		writer.BeginArray(0);
		writer.EndArray();

		u64 entityCount = 0;

		if (reader.Key("entityCount"))
			entityCount = reader.ReadU64();

		writer.Key("entityCount");
		writer.WriteU64(entityCount);

		Serializer serializer(request.pReflection, ResolveSceneType);

		usize chunkCount = 0;
		const b8 hasChunks = reader.Key("chunks");

		if (hasChunks)
			chunkCount = reader.BeginArray();

		writer.Key("chunks");
		writer.BeginArray(chunkCount);

		for (usize i = 0; i < chunkCount; i++)
		{
			if (!TranscodeChunk(request.pReflection, serializer, typeNames, reader, writer))
				return false;
		}

		writer.EndArray();

		if (hasChunks)
			reader.EndArray();

		reader.EndObject();
		writer.EndObject();

		if (reader.HasError())
		{
			Terminal::Error(StringOps::GetName(this), "{} is malformed", request.sourcePath);
			return false;
		}

		if (!writer.IsComplete())
		{
			Terminal::Error(StringOps::GetName(this), "{} produced an unbalanced payload", request.sourcePath);
			return false;
		}

		Engine::ContentFileWriter* pWriter = sink.Open(request.guid, request.assetTypeName);

		if (pWriter == nullptr)
		{
			Terminal::Error(StringOps::GetName(this), "{} has no content writer", request.sourcePath);
			return false;
		}

		const List<u8>& payload = writer.GetBytes();

		pWriter->SetPayloadVersion(Engine::SceneFormatVersion);
		pWriter->AddSection(Engine::SceneChunkSectionId, Engine::ContentSectionFlagResident, payload.GetData(),
			payload.GetCount());

		sink.Close(pWriter);

		return true;
	}
}