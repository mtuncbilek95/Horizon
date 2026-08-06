#include "ExitMenuItem.h"

#include <Engine/Core/Engine.h>

namespace Horizon
{
	void ExitMenuItem::OnExecute()
	{
		m_engine->RequestExit("File/Exit has been clicked!");
	}
}