#pragma once

#include <Engine/Core/Engine.h>
#include <Engine/World/ECS/Scene.h>
#include <Engine/World/WorldService.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	HCLASS();
	class H_EXPORT System : public Reflect::Base
	{
		friend class WorldService;
	public:
		virtual b8 OnInitialize() = 0;
		virtual void OnExecute(const EngineFrame& ctx, Scene& currentScene) = 0;
		virtual void OnFinalize() = 0;

		Engine* GetEngine() const { return m_engine; }
		WorldService* GetWorldService() const { return m_ownerService; }

		template<typename T>
		T* RequestSystem()
		{
			return m_ownerService->RequestSystem<T>();
		}

		System* RequestSystem(Reflect::TypeHandle handl)
		{
			return m_ownerService->RequestSystem(handl);
		}

	private:
		WorldService* m_ownerService;
		Engine* m_engine;
	};
}