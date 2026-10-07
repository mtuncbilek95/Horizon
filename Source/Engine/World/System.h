#pragma once

#include <Engine/Core/Engine.h>
#include <Engine/World/SystemGroup.h>
#include <Engine/World/ECS/Scene.h>
#include <Engine/World/WorldService.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	HCLASS();
	class ENGINE_API System : public Reflect::Base
	{
		friend class WorldService;
	public:
		virtual b8 OnInitialize() = 0;
		virtual void OnExecute(const EngineFrame& ctx, Scene& currentScene) = 0;
		virtual void OnFinalize() = 0;

		virtual void OnGroupsChanged(SystemGroup previous, SystemGroup current) = 0;
		virtual SystemGroup GetWorkingGroup() const = 0;

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