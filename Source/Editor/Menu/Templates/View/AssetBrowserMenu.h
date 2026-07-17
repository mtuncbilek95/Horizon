#pragma once

#include <Editor/Menu/IMenuItem.h>
#include <Editor/Menu/MenuAttribute.h>
#include "AssetBrowserMenu.reflected.h"

namespace Horizon
{
	HCLASS(MenuItemAttribute["View/Widgets/Asset Browser", 1u]);
	class AssetBrowserMenu : public IMenuItem
	{
		HORIZON_REFLECT;
	public:
		void OnInvoke() final;
	};
}