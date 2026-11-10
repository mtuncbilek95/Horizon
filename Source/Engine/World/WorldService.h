#pragma once

#include <Engine/Core/Service.h>
#include <Engine/World/Blackboard.h>
#include <Engine/World/SystemGraph.h>
#include <Runtime/Containers/Memory/UniquePtr.h>
#include <Runtime/Containers/Vector/List.h>
#include <Runtime/RTTR/Reflection.h>

#include <unordered_map>

namespace Horizon::Engine
{
	class Scene;
	class System;

	class WorldService : public Service
	{
	public:
		ModuleReport OnInitialize() final;
		void OnExecute(const EngineFrame& ctx) final;
		void OnFinalize() final;
		void DeclareDependencies(ModuleDependency& graph) final;

		Scene* GetCurrentWorld() const { return m_activeWorld; }
		void SetCurrentWorld(Scene* pNewWorld) { m_activeWorld = pNewWorld; }

		System* FindSystem(Reflect::TypeHandle handl);
		
		Blackboard& GetBlackboard() { return m_blackboard; }
		const Blackboard& GetBlackboard() const { return m_blackboard; }

	private:
		UniquePtr<SystemGraph> m_graph;

		Blackboard m_blackboard;
		Scene* m_activeWorld = nullptr;
	};
}