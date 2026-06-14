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
	}

	void PresentModule::OnDetach()
	{}
}