#include "SceneSerializer.h"

#include <Engine/Reflection/ReflectionSystem.h>
#include <Engine/World/ECS/Scene.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/Serialization/Serializer.h>

#include <string>

namespace Horizon::Asset
{
	void SceneSerializer::Serialize(const Engine::Scene& scene, Engine::ReflectionSystem* pReflection, IArchiveWriter& writer)
	{
		Serializer serializer(pReflection, ResolveType, ResolveTypeByName);

		const Engine::EntityStorage& entities = scene.GetEntities();
		const List<Engine::ComponentStorage*> storages = SortedStorages(scene);

		writer.BeginObject();

		writer.Key("version");
		writer.WriteU64(Version);

		writer.Key("entities");
		writer.BeginArray(CountSerializableEntities(scene));

		for (u32 index = 0; index < entities.GetLatestIndex(); index++)
		{
			const Engine::EntityHandle handle = entities.GetHandleAt(index);

			if (!IsSerializable(scene, handle))
				continue;

			writer.BeginObject();

			writer.Key("id");
			writer.WriteU64(handle.Index());

			writer.Key("components");
			writer.BeginArray(CountComponentsOf(storages, handle));

			for (const Engine::ComponentStorage* pStorage : storages)
			{
				const Engine::ComponentObject* pComponent = pStorage->Find(handle);

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

	b8 SceneSerializer::Deserialize(Engine::Scene& scene, Engine::ReflectionSystem* pReflection, IArchiveReader& reader)
	{
		Serializer serializer(pReflection, ResolveType, ResolveTypeByName);
		EntityRemap remap;

		reader.BeginObject();

		if (reader.Key("version") && reader.ReadU64() > Version)
		{
			Terminal::Error("SceneSerializer", "Engine::Scene archive is newer than the supported version {}", Version);
			reader.EndObject();
			return false;
		}

		if (!reader.Key("entities"))
		{
			Terminal::Error("SceneSerializer", "Engine::Scene archive carries no entity list");
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

			const Engine::EntityHandle handle = scene.AddEntity();
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

					Engine::ComponentObject* pComponent = scene.AddComponent(handle, pType->GetTypeId());

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

		RemapEntityFields(scene, pReflection, remap);

		return !reader.HasError();
	}

	const Reflect::Type* SceneSerializer::ResolveType(void* pUserData, Reflect::TypeHandle handle)
	{
		return static_cast<Engine::ReflectionSystem*>(pUserData)->GetType(handle);
	}

	const Reflect::Type* SceneSerializer::ResolveTypeByName(void* pUserData, const std::string& name)
	{
		return static_cast<Engine::ReflectionSystem*>(pUserData)->GetTypeByName(name);
	}

	b8 SceneSerializer::IsSerializable(const Engine::Scene& scene, Engine::EntityHandle handle)
	{
		if (!scene.IsAlive(handle))
			return false;

		return true;
	}

	usize SceneSerializer::CountSerializableEntities(const Engine::Scene& scene)
	{
		const Engine::EntityStorage& entities = scene.GetEntities();
		usize count = 0;

		for (u32 index = 0; index < entities.GetLatestIndex(); index++)
		{
			if (IsSerializable(scene, entities.GetHandleAt(index)))
				count++;
		}

		return count;
	}

	List<Engine::ComponentStorage*> SceneSerializer::SortedStorages(const Engine::Scene& scene)
	{
		List<Engine::ComponentStorage*> storages = scene.GetComponents().GetStorages();

		storages.Sort([](const Engine::ComponentStorage* pLeft, const Engine::ComponentStorage* pRight)
			{
				return pLeft->GetType()->GetName() < pRight->GetType()->GetName();
			});

		return storages;
	}

	usize SceneSerializer::CountComponentsOf(const List<Engine::ComponentStorage*>& storages, Engine::EntityHandle handle)
	{
		usize count = 0;

		for (const Engine::ComponentStorage* pStorage : storages)
		{
			if (pStorage->Find(handle) != nullptr)
				count++;
		}

		return count;
	}

	void SceneSerializer::RemapHandle(Engine::EntityHandle& handle, const EntityRemap& remap)
	{
		if (!handle.IsValid())
			return;

		auto it = remap.find(handle.Index());

		if (it == remap.end())
		{
			Terminal::Warn("SceneSerializer", "Entity id {} does not exist in the archive, link dropped", handle.Index());
			handle = Engine::EntityHandle();
			return;
		}

		handle = it->second;
	}

	void SceneSerializer::RemapObject(void* pObject, const Reflect::Type& type, Engine::ReflectionSystem* pReflection, const EntityRemap& remap)
	{
		for (const Reflect::Field& field : type.GetFields())
		{
			if (field.GetMode() == Reflect::TypeMode::Pointer)
			{
				Reflect::Base* pTarget = field.GetValueAs<Reflect::Base*>(pObject);

				if (pTarget == nullptr)
					continue;

				const Reflect::Type* pTargetType = pReflection->GetType(pTarget->GetTypeId());

				if (pTargetType == nullptr)
					continue;

				RemapObject(pTarget, *pTargetType, pReflection, remap);
				continue;
			}

			if (field.GetTypeId() != Reflect::TypeOf<Engine::EntityHandle>())
				continue;

			if (field.GetMode() != Reflect::TypeMode::Array)
			{
				RemapHandle(field.GetValueAs<Engine::EntityHandle>(pObject), remap);
				continue;
			}

			for (Engine::EntityHandle& handle : field.GetValueAs<List<Engine::EntityHandle>>(pObject))
				RemapHandle(handle, remap);
		}
	}

	void SceneSerializer::RemapEntityFields(Engine::Scene& scene, Engine::ReflectionSystem* pReflection, const EntityRemap& remap)
	{
		for (Engine::ComponentStorage* pStorage : scene.GetComponents().GetStorages())
		{
			for (usize i = 0; i < pStorage->GetCount(); i++)
				RemapObject(pStorage->GetAt(i), *pStorage->GetType(), pReflection, remap);
		}
	}
}