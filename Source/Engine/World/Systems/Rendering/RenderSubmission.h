#pragma once

#include <Engine/World/System.h>
#include <Engine/Rendering/RenderGraph/RenderGraph.h>
#include <Runtime/Containers/Memory/UniquePtr.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::World::System
{
	HCLASS();
	class ENGINE_API RenderSubmission : public Engine::System
	{
		HORIZON_TYPE_REFLECT(RenderSubmission);
	public:
		b8 OnInitialize() final;
		void OnExecute(const Engine::EngineFrame& ctx, Engine::Scene& currentScene) final;
		void OnFinalize() final;
		void DeclareDependencies(Engine::SystemDependency& graph) final;

	private:
		UniquePtr<Engine::RenderGraph> m_graph = nullptr;


	};
}