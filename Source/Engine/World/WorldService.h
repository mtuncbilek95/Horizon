#pragma once

#include <Engine/Core/Service.h>
#include <Engine/World/SystemGroup.h>
#include <Runtime/Containers/List.h>
#include <Runtime/RTTR/Reflection.h>

#include <unordered_map>
#include <string>

namespace Horizon::Engine
{
	class ReflectionSystem;
	class System;
	class Scene;

	class ENGINE_API WorldService : public Service
	{
		struct SystemEntry
		{
			System* pSystem = nullptr;
			const Reflect::Type* pType = nullptr;
			u32 order = 0;
		};
	public:
		ModuleReport OnInitialize() final;
		void OnExecute(const EngineFrame& ctx) final;
		void OnFinalize() final;
		void DeclareDependencies(ModuleGraph& graph) final;

		void OnLibraryRegistered(const ReflectionLibrary& library) final;
		void OnLibraryUnregistered(const ReflectionLibrary& library) final;

		template<typename T>
		T* RequestSystem() const
		{
			return (T*)RequestSystem(Reflect::TypeOf<T>());
		}
		System* RequestSystem(Reflect::TypeHandle handl) const;

		Scene* GetCurrentWorld() const { return m_activeWorld; }
		void SetCurrentWorld(Scene* pNewWorld) { m_activeWorld = pNewWorld; }

		SystemGroup GetRunningSystems() const { return m_runningSystems; }
		void SetRunningSystems(SystemGroup groups);

	private:
		b8 AddSystemType(const Reflect::Type* pType);
		void RemoveSystemType(const Reflect::Type* pType);
		void SortSystems();

	private:
		ReflectionSystem* m_reflection;
		List<SystemEntry> m_systems;
		std::unordered_map<Reflect::TypeHandle, System*> m_systemLookup;

		Scene* m_activeWorld = nullptr;
		SystemGroup m_runningSystems = SystemGroup::None;
	};
}