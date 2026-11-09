#pragma once

#include <Engine/World/ECS/Definitions.h>
#include <Runtime/Containers/Vector/List.h>
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
}

namespace Horizon::Asset
{
	class SceneSerializer
	{
		using EntityRemap = std::unordered_map<u32, Engine::EntityHandle>;
	public:
		static constexpr u32 Version = 1;

		static void Serialize(const Engine::Scene& scene, Engine::ReflectionSystem* pReflection, IArchiveWriter& writer);
		static b8 Deserialize(Engine::Scene& scene, Engine::ReflectionSystem* pReflection, IArchiveReader& reader);

	private:
		static const Reflect::Type* ResolveType(void* pUserData, Reflect::TypeHandle handle);
		static const Reflect::Type* ResolveTypeByName(void* pUserData, const std::string& name);

		static b8 IsSerializable(const Engine::Scene& scene, Engine::EntityHandle handle);
		static usize CountSerializableEntities(const Engine::Scene& scene);

		static List<Engine::ComponentStorage*> SortedStorages(const Engine::Scene& scene);
		static usize CountComponentsOf(const List<Engine::ComponentStorage*>& storages, Engine::EntityHandle handle);

		static void RemapHandle(Engine::EntityHandle& handle, const EntityRemap& remap);
		static void RemapObject(void* pObject, const Reflect::Type& type, Engine::ReflectionSystem* pReflection, const EntityRemap& remap);
		static void RemapEntityFields(Engine::Scene& scene, Engine::ReflectionSystem* pReflection, const EntityRemap& remap);
	};
}