#include "FileMenu.h"

#include <Editor/Menu/MenuDefinitions.h>

#include <Engine/Core/Engine.h>

namespace Horizon
{
	void ExitEngineMenu::OnInvoke()
	{
		GetEngine()->RequestExit("File/Exit clicked");
	}
}

HORIZON_BIND_MENU(ExitEngineMenu, "File/Exit", 100);