#pragma once

#include <Engine/Core/Engine.h>
#include <Engine/World/ECS/Scene.h>
#include <Engine/World/SystemDependency.h>
#include <Engine/World/WorldService.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	HCLASS();
	class ENGINE_API System : public Reflect::Base
	{
		friend class SystemGraph;
	public:
		virtual b8 OnInitialize() = 0;
		virtual void OnExecute(const EngineFrame& ctx, Scene& currentScene) = 0;
		virtual void OnFinalize() = 0;
		virtual void DeclareDependencies(SystemDependency& graph) = 0;

		Engine* GetEngine() const { return m_engine; }

	private:
		Engine* m_engine;
	};
}