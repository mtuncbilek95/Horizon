#pragma once

#include <Engine/ECS/ECS.h>

namespace Horizon
{
	class GraphicsSystem;

	class MeshComponentSystem : public ECS<MeshComponentSystem>
	{
	public:
		MeshComponentSystem() = default;
		virtual ~MeshComponentSystem() = default;

		SystemReport OnStart() final;
		void OnTick() final;
		void OnStop() final;

	private:
		GraphicsSystem* m_relatedGraphics = nullptr;
	};
}