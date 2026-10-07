#pragma once

#include <Runtime/Containers/List.h>
#include <string>

namespace Horizon::Editor
{
	class MenuItem;

	struct EDITOR_API MenuItemInstance
	{
		std::string displayName;
		i32 order;
		b8 isCheckbox;

		MenuItem* pMenu;

		List<MenuItemInstance> subMenus;
	};
}