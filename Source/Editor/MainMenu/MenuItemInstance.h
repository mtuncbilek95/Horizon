#pragma once

#include <Runtime/Containers/List.h>

#include <string>

namespace Horizon::Reflect
{
	class Type;
}

namespace Horizon::Editor
{
	class MenuItem;

	struct EDITOR_API MenuItemInstance
	{
		std::string displayName;
		i32 order = 0;
		b8 isCheckbox = false;

		MenuItem* pMenu = nullptr;
		const Reflect::Type* pType = nullptr;

		List<MenuItemInstance> subMenus;
	};
}