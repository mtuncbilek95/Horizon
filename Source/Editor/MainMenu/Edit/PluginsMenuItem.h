#pragma once

#include <Editor/MainMenu/MenuItemAttribute.h>
#include <Editor/MainMenu/MenuItem.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Editor
{
	HCLASS(MenuItem["Edit/Plugins", i32_max - 1]);
	class EDITOR_API PluginsMenuItem : public MenuItem
	{
		HORIZON_TYPE_REFLECT(PluginsMenuItem);
	public:
		void OnExecute() final;
	};
}