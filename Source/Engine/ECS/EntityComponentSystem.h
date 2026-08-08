#pragma once

#include <Engine/Core/System.h>

namespace Horizon::Engine
{
	class WorldAsset;
	class WorldRegistry;

	class EntityComponentSystem : public System
	{
	public:
		AppReport OnAttach(Application* engine) final;
		void OnDetach() final;

		void GetInitializeOrder(OrderRules& rules) const final;
		void GetExecutionOrder(OrderRules& rules) const final;

		void SetActiveWorld(WorldAsset* world) { m_activeWorld = world; }
		WorldAsset* GetActiveWorld() const { return m_activeWorld; }

	private:
		WorldAsset* m_activeWorld = nullptr;
	};
}