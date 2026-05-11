#pragma once

#include <Engine/Core/System.h>

namespace Horizon
{
	class DummySystem : public System<DummySystem>
	{
	public:

	private:
		EngineReport OnInitialize() final;
		void OnSync() final;
		void OnFinalize() final;
	};
}