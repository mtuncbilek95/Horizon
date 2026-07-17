#pragma once

#include <Editor/Menu/MenuAttribute.h>
#include "EditorMainMenu.reflected.h"

namespace Horizon
{
	HCLASS(MainMenuAttribute["File", 10u],
		MainMenuAttribute["Edit", 20u],
		MainMenuAttribute["View", 40u],
		MainMenuAttribute["View/Widgets", 1u]);
	struct EditorMainMenu
	{
		HORIZON_REFLECT;
	};
}