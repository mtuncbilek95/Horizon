#pragma once

#include <Engine/Core/Subsystem.h>

namespace Horizon
{
	class H_EXPORT AssetSubsystem : public Subsystem
	{
	public:
		AssetSubsystem() = default;
		~AssetSubsystem() = default;

		EngineReport OnAttach(Engine* pEngine);
		void OnSync();
		void OnDetach();

		void GetInitializeOrder(OrderRules& rules) const final;
	};
}