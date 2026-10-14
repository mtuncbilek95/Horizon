#pragma once

#include <Editor/MainMenu/MenuItemAttribute.h>
#include <Editor/MainMenu/MenuItem.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Editor
{
	HCLASS(MenuItem["Edit/Project Settings", i32_max]);
	class EDITOR_API ProjectSettingsMenuItem : public MenuItem
	{
		HORIZON_TYPE_REFLECT(ProjectSettingsMenuItem);
	public:
		void OnExecute() final;
	};
}