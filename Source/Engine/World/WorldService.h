#pragma once

#include <Engine/Core/Service.h>
#include <Runtime/Containers/Memory/UniquePtr.h>
#include <Runtime/Containers/Vector/List.h>
#include <Runtime/RTTR/Reflection.h>

#include <unordered_map>

namespace Horizon::Engine
{
	class SystemGraph;
	class Scene;

	class ENGINE_API WorldService : public Service
	{
	public:
		ModuleReport OnInitialize() final;
		void OnExecute(const EngineFrame& ctx) final;
		void OnFinalize() final;
		void DeclareDependencies(ModuleGraph& graph) final;

		void OnLibraryRegistered(const ReflectionLibrary& library) final;
		void OnLibraryUnregistered(const ReflectionLibrary& library) final;

		Scene* GetCurrentWorld() const { return m_activeWorld; }
		void SetCurrentWorld(Scene* pNewWorld) { m_activeWorld = pNewWorld; }

	private:
		UniquePtr<SystemGraph> m_graph;

		Scene* m_activeWorld = nullptr;
	};
}