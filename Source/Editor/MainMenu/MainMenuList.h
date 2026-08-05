#pragma once

#include <Editor/Attributes/MainMenuItemAttribute.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon
{
	HCLASS(MainMenuItem["File", 0]);
	class H_EXPORT FileMenu {};

	HCLASS(MainMenuItem["Edit", 100]);
	class H_EXPORT EditMenu {};

	HCLASS(MainMenuItem["View", 200]);
	class H_EXPORT ViewMenu {};

	HCLASS(MainMenuItem["Assets", 300]);
	class H_EXPORT AssetsMenu {};

	HCLASS(MainMenuItem["Build", 400]);
	class H_EXPORT BuildMenu {};

	HCLASS(MainMenuItem["Help", 500]);
	class H_EXPORT HelpMenu {};
}