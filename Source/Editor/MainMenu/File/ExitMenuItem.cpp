#include "ExitMenuItem.h"

#include <Engine/Core/Engine.h>

namespace Horizon::Editor
{
	void ExitMenuItem::OnExecute()
	{
		GetEngine()->RequestExit("File/Exit has been clicked!");
	}
}