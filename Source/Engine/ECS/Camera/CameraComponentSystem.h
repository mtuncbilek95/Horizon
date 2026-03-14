#pragma once

#include <Engine/ECS/ECS.h>

namespace Horizon
{
	class CameraComponentSystem : public ECS<CameraComponentSystem>
	{
	public:
		CameraComponentSystem() = default;
		virtual ~CameraComponentSystem() = default;

		SystemReport OnStart() final;
		void OnTick() final;
		void OnStop() final;
	};
}