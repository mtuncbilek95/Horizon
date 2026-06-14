#include "EditorModule.h"

#include <Engine/Engine/Engine.h>
#include <Engine/Window/WindowModule.h>
#include <Engine/Graphics/GraphicsModule.h>

namespace Horizon
{
	void EditorModule::OnAttach(Engine& engine)
	{
		IModule::OnAttach(engine);
	}

	void EditorModule::OnSync()
	{
	}

	void EditorModule::OnDetach()
	{}
}