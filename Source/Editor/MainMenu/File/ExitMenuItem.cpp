#include "ExitMenuItem.h"

#include <Engine/Core/Application.h>

namespace Horizon::Editor
{
	void ExitMenuItem::OnExecute()
	{
		m_engine->RequestExit("File/Exit has been clicked!");
	}
}