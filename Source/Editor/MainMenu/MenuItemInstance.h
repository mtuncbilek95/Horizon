#pragma once

#include <Runtime/Containers/List.h>
#include <string>

namespace Horizon
{
	class MenuItem;

	struct H_EXPORT MenuItemInstance
	{
		std::string displayName;
		i32 order;
		b8 isCheckbox;

		MenuItem* menu;

		List<MenuItemInstance> subMenus;
	};
}