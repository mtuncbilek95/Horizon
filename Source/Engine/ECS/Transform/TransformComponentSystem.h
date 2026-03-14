#pragma once

#include <Engine/ECS/ECS.h>

namespace Horizon
{
	class TransformComponentSystem : public ECS<TransformComponentSystem>
	{
	public:
		TransformComponentSystem() = default;
		virtual ~TransformComponentSystem() = default;

		SystemReport OnStart() final;
		void OnTick() final;
		void OnStop() final;
	};
}