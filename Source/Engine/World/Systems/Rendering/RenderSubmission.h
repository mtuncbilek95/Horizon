#pragma once

#include <Engine/World/System.h>
#include <Engine/Rendering/RenderGraph/RenderGraph.h>
#include <Engine/Rendering/RenderDrawContext.h>
#include <Runtime/Containers/Memory/UniquePtr.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	class PresentationService;
	class AssetService;
}

namespace Horizon::World::System
{
	HCLASS();
	class RenderSubmission : public Engine::System
	{
		HORIZON_TYPE_REFLECT(RenderSubmission);
	public:
		b8 OnInitialize() final;
		void OnExecute(const Engine::EngineFrame& ctx, Engine::Scene& currentScene) final;
		void OnFinalize() final;
		void DeclareDependencies(Engine::SystemDependency& graph) final;

		Engine::RenderGraph* GetGraph() const { return m_graph.Get(); }

	private:
		UniquePtr<Engine::RenderGraph> m_graph = nullptr;
		Engine::PresentationService* m_presentation = nullptr;
		Engine::AssetService* m_assets = nullptr;

		Engine::RenderDrawContext m_drawList;
	};
}