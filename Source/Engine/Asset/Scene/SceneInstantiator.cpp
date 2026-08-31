#include "SceneInstantiator.h"

#include <Engine/Reflection/ReflectionSystem.h>

#include <Runtime/Containers/StringOps.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/Serialization/BinaryArchive.h>
#include <Runtime/Serialization/Serializer.h>

#include <cstring>

namespace Horizon::Engine
{
	namespace
	{
		const Reflect::Type* ResolveSceneType(void* pUserData, Reflect::TypeHandle handle)
		{
			return static_cast<ReflectionSystem*>(pUserData)->GetType(handle);
		}
	}

	b8 SceneInstantiator::Apply(const SceneAsset& scene, World& world, ReflectionSystem* pReflection)
	{
		if (!pReflection)
		{
			Terminal::Error("SceneInstantiator", "No reflection system was supplied");
			return false;
		}

		const List<u8>& blob = scene.GetBlob();

		BinaryArchiveReader reader(blob.GetData(), blob.GetCount());

		if (reader.HasError())
			return false;

		List<EntityHandle> remap;

		if (!world.GetEntities().ReserveRange(scene.GetEntityCount(), remap))
			return false;

		Serializer serializer(pReflection, ResolveSceneType);

		for (const SceneChunkDesc& chunk : scene.GetChunks())
		{
			if (chunk.typeIndex >= scene.GetTypeNames().GetCount())
			{
				Terminal::Error("SceneInstantiator", "A chunk names the unknown type index {}", chunk.typeIndex);
				continue;
			}

			const std::string& typeName = scene.GetTypeNames()[chunk.typeIndex];
			Reflect::Type* pType = pReflection->GetTypeByName(typeName);

			if (!pType)
			{
				Terminal::Error("SceneInstantiator", "'{}' is not registered in this build", typeName);
				continue;
			}

			const b8 blittable = (chunk.flags & SceneChunkFlagBlittable) != 0;

			if (blittable && chunk.stride != pType->GetSizeInBytes())
			{
				Terminal::Error("SceneInstantiator", "'{}' is {} bytes on disk but {} in this build", typeName, chunk.stride, pType->GetSizeInBytes());
				continue;
			}

			ComponentStorage* pStorage = world.GetComponents().GetOrCreateStorage(*pType);

			if (!pStorage)
				continue;

			const usize baseIndex = pStorage->GetCount();
			pStorage->ResizeDense(baseIndex + chunk.count);

			if (blittable)
			{
				std::memcpy(static_cast<u8*>(pStorage->GetDenseData()) + baseIndex * chunk.stride,
					blob.GetData() + chunk.dataOffset, chunk.dataSize);
			}
			else
			{
				if (!reader.Seek(chunk.dataOffset))
					continue;

				const usize objectCount = reader.BeginArray();

				for (usize i = 0; i < objectCount && i < chunk.count; i++)
					serializer.Deserialize(pStorage->GetAt(baseIndex + i), *pType, reader);

				reader.EndArray();
			}

			const u32* pDiskIndices = reinterpret_cast<const u32*>(blob.GetData() + chunk.entityOffset);

			for (usize i = 0; i < chunk.count; i++)
			{
				const u32 diskIndex = pDiskIndices[i];

				if (diskIndex >= remap.GetCount())
				{
					Terminal::Error("SceneInstantiator", "A chunk references the entity {} outside the scene", diskIndex);
					continue;
				}

				const EntityHandle entity = remap[diskIndex];

				pStorage->BindEntityAt(baseIndex + i, entity);
				world.MarkComponent(entity, pStorage->GetSlot());
			}
		}

		return !reader.HasError();
	}

	b8 SceneInstantiator::Capture(World& world, ReflectionSystem* pReflection, IArchiveWriter& writer)
	{
		if (!pReflection)
		{
			Terminal::Error("SceneInstantiator", "No reflection system was supplied");
			return false;
		}

		EntityStorage& entities = world.GetEntities();
		const u32 highWater = entities.GetHighWaterMark();

		List<u32> diskIndices(highWater, InvalidDenseIndex);
		u32 entityCount = 0;

		for (u32 i = 0; i < highWater; i++)
		{
			if (!entities.IsAlive(entities.GetHandleAt(i)))
				continue;

			diskIndices[i] = entityCount;
			entityCount++;
		}

		List<ComponentStorage*> chunks;

		for (ComponentStorage* pStorage : world.GetComponents().GetStorages())
		{
			if (pStorage->GetCount() == 0)
				continue;

			chunks.PushBack(pStorage);
		}

		Serializer serializer(pReflection, ResolveSceneType);

		writer.BeginObject();

		writer.Key("version");
		writer.WriteU64(SceneFormatVersion);

		writer.Key("entityCount");
		writer.WriteU64(entityCount);

		writer.Key("typeNames");
		writer.BeginArray(chunks.GetCount());

		for (ComponentStorage* pStorage : chunks)
			writer.WriteString(pStorage->GetType().GetName());

		writer.EndArray();

		writer.Key("chunks");
		writer.BeginArray(chunks.GetCount());

		for (usize typeIndex = 0; typeIndex < chunks.GetCount(); typeIndex++)
		{
			ComponentStorage* pStorage = chunks[typeIndex];

			List<usize> denseIndices;

			for (usize i = 0; i < pStorage->GetCount(); i++)
			{
				const u32 entityIndex = (u32)pStorage->GetEntityAt(i).Index();

				if (entityIndex >= diskIndices.GetCount() || diskIndices[entityIndex] == InvalidDenseIndex)
				{
					Terminal::Warn("SceneInstantiator", "'{}' holds a component for a dead entity, it is dropped",
						pStorage->GetType().GetName());
					continue;
				}

				denseIndices.PushBack(i);
			}

			writer.BeginObject();

			writer.Key("typeIndex");
			writer.WriteU64(typeIndex);

			writer.Key("entities");
			writer.BeginArray(denseIndices.GetCount());

			for (usize denseIndex : denseIndices)
				writer.WriteU64(diskIndices[(u32)pStorage->GetEntityAt(denseIndex).Index()]);

			writer.EndArray();

			writer.Key("objects");
			writer.BeginArray(denseIndices.GetCount());

			for (usize denseIndex : denseIndices)
				serializer.Serialize(pStorage->GetAt(denseIndex), pStorage->GetType(), writer);

			writer.EndArray();
			writer.EndObject();
		}

		writer.EndArray();
		writer.EndObject();

		return true;
	}

	void SceneInstantiator::CaptureEmpty(IArchiveWriter& writer)
	{
		writer.BeginObject();

		writer.Key("version");
		writer.WriteU64(SceneFormatVersion);

		writer.Key("entityCount");
		writer.WriteU64(0);

		writer.Key("typeNames");
		writer.BeginArray(0);
		writer.EndArray();

		writer.Key("chunks");
		writer.BeginArray(0);
		writer.EndArray();

		writer.EndObject();
	}
}