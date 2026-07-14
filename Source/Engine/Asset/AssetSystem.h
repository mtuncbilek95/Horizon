#pragma once

#include <Engine/Core/System.h>

namespace Horizon
{
	class H_EXPORT AssetSystem : public System
	{
	public:
		AssetSystem() = default;
		~AssetSystem() = default;

		EngineReport OnAttach(Engine* pEngine);
		void OnSync();
		void OnDetach();

		void GetInitializeOrder(OrderRules& rules) const final;
	};
}