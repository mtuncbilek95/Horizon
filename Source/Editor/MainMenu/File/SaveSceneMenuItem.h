#pragma once

#include <Editor/Attributes/MenuItemAttribute.h>
#include <Editor/MainMenu/MenuItem.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Editor
{
	HCLASS(MenuItem["File/Save Scene", 10]);
	class H_EXPORT SaveSceneMenuItem : public MenuItem
	{
		HORIZON_TYPE_REFLECT(SaveSceneMenuItem);
	public:
		void OnExecute() final;
	};
}