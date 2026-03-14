#include "MeshComponentSystem.h"

#include <Runtime/Graphics/RHI/Command/GfxCommandBuffer.h>

#include <Engine/Graphics/GraphicsSystem.h>
#include <Engine/ECS/Mesh/MeshComponent.h>

namespace Horizon
{
	SystemReport MeshComponentSystem::OnStart()
	{
		// Log::Terminal(LogType::Success, GetObjectType(), "MeshComponentSystem started.");
		return SystemReport();
	}

	void MeshComponentSystem::OnTick()
	{
		// Log::Terminal(LogType::Debug, GetObjectType(), "MeshComponentSystem tick.");

		auto& graphicsSystem = RequestSystem<GraphicsSystem>();
		/*auto view = GetRegistry().view<MeshComponent>();
		view.each([&](auto entity, auto& meshComponent)
			{
			});*/
	}

	void MeshComponentSystem::OnStop()
	{
		// Log::Terminal(LogType::Warning, GetObjectType(), "MeshComponentSystem stopped.");
	}
}