#include "PresentModule.h"

#include <Engine/Engine/Engine.h>
#include <Engine/Graphics/GraphicsModule.h>

namespace Horizon
{
	void PresentModule::OnAttach(Engine& engine)
	{
		IModule::OnAttach(engine);
	}

	void PresentModule::OnSync()
	{
		auto& graphics = m_engine->GetModule<GraphicsModule>();

		graphics.EndFrame();
	}

	void PresentModule::OnDetach()
	{}
}