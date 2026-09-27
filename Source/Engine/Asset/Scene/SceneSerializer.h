#pragma once

#include <Engine/World/ECS/Definitions.h>
#include <Runtime/Containers/List.h>
#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/RTTR/Reflection.h>
#include <Runtime/Serialization/Archive.h>

#include <unordered_map>

namespace Horizon::Engine
{
	class ComponentStorage;
	class ReflectionSystem;
	class Scene;

	class H_EXPORT SceneSerializer
	{
		using EntityRemap = std::unordered_map<u32, EntityHandle>;
	public:
		static constexpr u32 Version = 1;

		static void Serialize(const Scene& scene, ReflectionSystem* pReflection, IArchiveWriter& writer);
		static b8 Deserialize(Scene& scene, ReflectionSystem* pReflection, IArchiveReader& reader);

	private:
		static const Reflect::Type* ResolveType(void* pUserData, Reflect::TypeHandle handle);

		static List<ComponentStorage*> SortedStorages(const Scene& scene);
		static usize CountComponentsOf(const List<ComponentStorage*>& storages, EntityHandle handle);

		static List<const Reflect::Field*> EntityFieldsOf(const Reflect::Type* pType);
		static void RemapHandle(EntityHandle& handle, const EntityRemap& remap);
		static void RemapComponent(void* pComponent, const List<const Reflect::Field*>& fields, const EntityRemap& remap);
		static void RemapEntityFields(Scene& scene, const EntityRemap& remap);
	};
}