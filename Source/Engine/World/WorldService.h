#pragma once

#include <Engine/Core/Service.h>
#include <Runtime/Containers/List.h>
#include <Runtime/RTTR/Reflection.h>

#include <unordered_map>
#include <string>

namespace Horizon::Engine
{
	class ReflectionSystem;
	class System;
	class Scene;

	class H_EXPORT WorldService : public Service
	{
		struct SystemEntry
		{
			System* pSystem = nullptr;
			std::string name;
			u32 order = 0;
		};
	public:
		ModuleReport OnInitialize() final;
		void OnExecute(const EngineFrame& ctx) final;
		void OnFinalize() final;
		void DeclareDependencies(ModuleGraph& graph) final;  

		template<typename T>
		T* RequestSystem() const
		{
			return (T*)RequestSystem(Reflect::TypeOf<T>());
		}
		System* RequestSystem(Reflect::TypeHandle handl) const;

	private:
		ReflectionSystem* m_reflection;
		List<System*> m_systems;
		std::unordered_map<Reflect::TypeHandle, usize> m_systemLookup;

		Scene* m_activeWorld = nullptr;
	};
}