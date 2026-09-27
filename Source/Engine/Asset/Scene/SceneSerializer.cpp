#include "SceneSerializer.h"

#include <Engine/Reflection/ReflectionSystem.h>
#include <Engine/World/ECS/Scene.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/Serialization/Serializer.h>

#include <string>

namespace Horizon::Engine
{
	void SceneSerializer::Serialize(const Scene& scene, ReflectionSystem* pReflection, IArchiveWriter& writer)
	{
		Serializer serializer(pReflection, ResolveType);

		const EntityStorage& entities = scene.GetEntities();
		const List<ComponentStorage*> storages = SortedStorages(scene);

		writer.BeginObject();

		writer.Key("version");
		writer.WriteU64(Version);

		writer.Key("entities");
		writer.BeginArray(entities.GetAliveCount());

		for (u32 index = 0; index < entities.GetLatestIndex(); index++)
		{
			const EntityHandle handle = entities.GetHandleAt(index);

			if (!entities.IsAlive(handle))
				continue;

			writer.BeginObject();

			writer.Key("id");
			writer.WriteU64(handle.Index());

			writer.Key("components");
			writer.BeginArray(CountComponentsOf(storages, handle));

			for (const ComponentStorage* pStorage : storages)
			{
				const ComponentObject* pComponent = pStorage->Find(handle);

				if (pComponent == nullptr)
					continue;

				const Reflect::Type* pType = pStorage->GetType();

				writer.BeginObject();

				writer.Key("type");
				writer.WriteString(pType->GetName());

				writer.Key("data");
				serializer.Serialize(pComponent, *pType, writer);

				writer.EndObject();
			}

			writer.EndArray();
			writer.EndObject();
		}

		writer.EndArray();
		writer.EndObject();
	}

	b8 SceneSerializer::Deserialize(Scene& scene, ReflectionSystem* pReflection, IArchiveReader& reader)
	{
		Serializer serializer(pReflection, ResolveType);
		EntityRemap remap;

		reader.BeginObject();

		if (reader.Key("version") && reader.ReadU64() > Version)
		{
			Terminal::Error("SceneSerializer", "Scene archive is newer than the supported version {}", Version);
			reader.EndObject();
			return false;
		}

		if (!reader.Key("entities"))
		{
			Terminal::Error("SceneSerializer", "Scene archive carries no entity list");
			reader.EndObject();
			return false;
		}

		const usize entityCount = reader.BeginArray();

		for (usize i = 0; i < entityCount; i++)
		{
			reader.BeginObject();

			u32 fileId = 0;

			if (reader.Key("id"))
				fileId = static_cast<u32>(reader.ReadU64());

			const EntityHandle handle = scene.AddEntity();
			remap[fileId] = handle;

			if (reader.Key("components"))
			{
				const usize componentCount = reader.BeginArray();

				for (usize j = 0; j < componentCount; j++)
				{
					reader.BeginObject();

					if (!reader.Key("type"))
					{
						Terminal::Warn("SceneSerializer", "Entity {} has a component without a type, skipped", fileId);
						reader.EndObject();
						continue;
					}

					const std::string typeName = reader.ReadString();
					const Reflect::Type* pType = pReflection->GetTypeByName(typeName);

					if (pType == nullptr)
					{
						Terminal::Warn("SceneSerializer", "{} is not a reflected component, skipped", typeName);
						reader.EndObject();
						continue;
					}

					ComponentObject* pComponent = scene.AddComponent(handle, pType->GetTypeId());

					if (pComponent == nullptr)
					{
						Terminal::Warn("SceneSerializer", "{} could not be added to entity {}, skipped", typeName, fileId);
						reader.EndObject();
						continue;
					}

					if (reader.Key("data"))
						serializer.Deserialize(pComponent, *pType, reader);

					reader.EndObject();
				}

				reader.EndArray();
			}

			reader.EndObject();
		}

		reader.EndArray();
		reader.EndObject();

		RemapEntityFields(scene, remap);

		return !reader.HasError();
	}

	const Reflect::Type* SceneSerializer::ResolveType(void* pUserData, Reflect::TypeHandle handle)
	{
		return static_cast<ReflectionSystem*>(pUserData)->GetType(handle);
	}

	List<ComponentStorage*> SceneSerializer::SortedStorages(const Scene& scene)
	{
		List<ComponentStorage*> storages = scene.GetComponents().GetStorages();

		storages.Sort([](const ComponentStorage* pLeft, const ComponentStorage* pRight)
			{
				return pLeft->GetType()->GetName() < pRight->GetType()->GetName();
			});

		return storages;
	}

	usize SceneSerializer::CountComponentsOf(const List<ComponentStorage*>& storages, EntityHandle handle)
	{
		usize count = 0;

		for (const ComponentStorage* pStorage : storages)
		{
			if (pStorage->Find(handle) != nullptr)
				count++;
		}

		return count;
	}

	List<const Reflect::Field*> SceneSerializer::EntityFieldsOf(const Reflect::Type* pType)
	{
		List<const Reflect::Field*> fields;

		for (const Reflect::Field& field : pType->GetFields())
		{
			if (field.GetTypeId() == Reflect::TypeOf<EntityHandle>())
				fields.PushBack(&field);
		}

		return fields;
	}

	void SceneSerializer::RemapHandle(EntityHandle& handle, const EntityRemap& remap)
	{
		if (!handle.IsValid())
			return;

		auto it = remap.find(handle.Index());

		if (it == remap.end())
		{
			Terminal::Warn("SceneSerializer", "Entity id {} does not exist in the archive, link dropped", handle.Index());
			handle = EntityHandle();
			return;
		}

		handle = it->second;
	}

	void SceneSerializer::RemapComponent(void* pComponent, const List<const Reflect::Field*>& fields, const EntityRemap& remap)
	{
		for (const Reflect::Field* pField : fields)
		{
			if (pField->GetMode() != Reflect::TypeMode::Array)
			{
				RemapHandle(pField->GetValueAs<EntityHandle>(pComponent), remap);
				continue;
			}

			for (EntityHandle& handle : pField->GetValueAs<List<EntityHandle>>(pComponent))
				RemapHandle(handle, remap);
		}
	}

	void SceneSerializer::RemapEntityFields(Scene& scene, const EntityRemap& remap)
	{
		for (ComponentStorage* pStorage : scene.GetComponents().GetStorages())
		{
			const List<const Reflect::Field*> fields = EntityFieldsOf(pStorage->GetType());

			if (fields.IsEmpty())
				continue;

			for (usize i = 0; i < pStorage->GetCount(); i++)
				RemapComponent(pStorage->GetAt(i), fields, remap);
		}
	}
}