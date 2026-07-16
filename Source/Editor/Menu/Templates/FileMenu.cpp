#include "FileMenu.h"

#include <Engine/Core/Engine.h>

namespace Horizon
{
	void ExitEngineMenu::OnInvoke()
	{
		GetEngine()->RequestExit("File/Exit clicked");
	}
}
