#include "EngineSettingsMenuItem.h"

#include <Runtime/Containers/StringOps.h>
#include <Runtime/Log/Terminal.h>

namespace Horizon::Editor
{
	void EngineSettingsMenuItem::OnExecute()
	{
		Terminal::Error(StringOps::GetName(this), "Not implemented yet");
	}
}