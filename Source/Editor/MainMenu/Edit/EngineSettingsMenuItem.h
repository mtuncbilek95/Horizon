#pragma once

#include <Editor/MainMenu/MenuItemAttribute.h>
#include <Editor/MainMenu/MenuItem.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Editor
{
	HCLASS(MenuItem["Edit/Engine Settings", i32_max]);
	class EDITOR_API EngineSettingsMenuItem : public MenuItem
	{
		HORIZON_TYPE_REFLECT(EngineSettingsMenuItem);
	public:
		void OnExecute() final;
	};
}