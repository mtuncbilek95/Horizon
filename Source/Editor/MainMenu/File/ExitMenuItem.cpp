#include "ExitMenuItem.h"

#include <Engine/Core/Engine.h>

namespace Horizon::Editor
{
	void ExitMenuItem::OnExecute()
	{
		GetContext()->pEngine->RequestExit("File/Exit has been clicked!");
	}
}