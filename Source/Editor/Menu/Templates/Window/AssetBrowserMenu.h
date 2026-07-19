#pragma once

#include <Editor/Menu/IMenuItem.h>
#include <Editor/Menu/MenuAttribute.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon
{
	HCLASS(MenuItemAttribute["Window/Panels/Asset Browser", 1u]);
	class AssetBrowserMenu : public IMenuItem
	{
		HORIZON_TYPE_REFLECT;

	public:
		void OnInvoke() final;
	};
}