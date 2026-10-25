#include "EngineSettingsMenuItem.h"

#include <Runtime/Containers/StringHelper.h>
#include <Runtime/Log/Terminal.h>

namespace Horizon::Editor
{
	void EngineSettingsMenuItem::OnExecute()
	{
		Terminal::Error(StringHelper::GetTypeName(this), "Not implemented yet");
	}
}