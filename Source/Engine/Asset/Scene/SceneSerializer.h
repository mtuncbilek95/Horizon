#pragma once

#include <Engine/World/ECS/Definitions.h>
#include <Runtime/Containers/List.h>
#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/RTTR/Reflection.h>
#include <Runtime/Serialization/Archive.h>

#include <string>
#include <unordered_map>

namespace Horizon::Engine
{
	class ComponentStorage;
	class ReflectionSystem;
	class Scene;

	class ENGINE_API SceneSerializer
	{
		using EntityRemap = std::unordered_map<u32, EntityHandle>;
	public:
		static constexpr u32 Version = 1;

		static void Serialize(const Scene& scene, ReflectionSystem* pReflection, IArchiveWriter& writer);
		static b8 Deserialize(Scene& scene, ReflectionSystem* pReflection, IArchiveReader& reader);

	private:
		static const Reflect::Type* ResolveType(void* pUserData, Reflect::TypeHandle handle);
		static const Reflect::Type* ResolveTypeByName(void* pUserData, const std::string& name);

		static b8 IsSerializable(const Scene& scene, EntityHandle handle);
		static usize CountSerializableEntities(const Scene& scene);

		static List<ComponentStorage*> SortedStorages(const Scene& scene);
		static usize CountComponentsOf(const List<ComponentStorage*>& storages, EntityHandle handle);

		static void RemapHandle(EntityHandle& handle, const EntityRemap& remap);
		static void RemapObject(void* pObject, const Reflect::Type& type, ReflectionSystem* pReflection, const EntityRemap& remap);
		static void RemapEntityFields(Scene& scene, ReflectionSystem* pReflection, const EntityRemap& remap);
	};
}