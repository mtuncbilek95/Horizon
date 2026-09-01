#pragma once

#include <Engine/World/System.h>

namespace Horizon::Engine
{
	HCLASS();
	class H_EXPORT RenderSystem : public System
	{
		HORIZON_TYPE_REFLECT(RenderSystem);
	public:
		b8 OnInitialize() final;
		void OnExecute() final;
		void OnFinalize() final;

	private:
	};
}