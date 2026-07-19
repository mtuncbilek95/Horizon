#include "ExitEngineMenu.h"

#include <Engine/Core/Engine.h>

namespace Horizon
{
	void ExitEngineMenu::OnInvoke()
	{
		GetEngine()->RequestExit("Editor's Exit button clicked!");
	}
}