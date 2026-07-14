#pragma once

#include <Engine/Core/System.h>

namespace Horizon
{
	class WorldRegistry;

	class EntityComponentSystem : public System
	{
	public:
		EngineReport OnAttach(Engine* engine) final;
		void OnDetach() final;

		void GetInitializeOrder(OrderRules& rules) const final;
		void GetExecutionOrder(OrderRules& rules) const final;

		WorldRegistry& GetWorld() { return *m_currentWorld; }

	private:
		WorldRegistry* m_currentWorld;
	};
}