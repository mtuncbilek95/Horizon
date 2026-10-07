#pragma once

#include <Editor/MainMenu/MenuItemAttribute.h>
#include <Editor/MainMenu/MenuItem.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::TestLibrary
{
	HCLASS(Editor::MenuItem["Plugins/TestLibrary", i32_max]);
	class TestViewMenuItem : public Editor::MenuItem
	{
		HORIZON_TYPE_REFLECT(TestViewMenuItem);
	public:
		void OnExecute() final;
	};
}