#pragma once

#include <Editor/Menu/IMenuItem.h>
#include <Editor/Menu/MenuAttribute.h>
#include "ExitEngineMenu.reflected.h"

namespace Horizon
{
	HCLASS(MenuItemAttribute["File/Exit", 100u]);
	class ExitEngineMenu : public IMenuItem
	{
		HORIZON_REFLECT;
	public:
		void OnInvoke() final;
	};
}