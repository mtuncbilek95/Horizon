#pragma once

#include <Editor/MainMenu/MainMenuItemAttribute.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Editor
{
	HCLASS(MainMenuItem["File", 0]);
	class FileMenu : public Reflect::Base
	{
		HORIZON_TYPE_REFLECT(FileMenu);
	};

	HCLASS(MainMenuItem["Edit", 100]);
	class EditMenu : public Reflect::Base
	{
		HORIZON_TYPE_REFLECT(EditMenu);
	};

	HCLASS(MainMenuItem["View", 200]);
	class ViewMenu : public Reflect::Base
	{
		HORIZON_TYPE_REFLECT(ViewMenu);
	};

	HCLASS(MainMenuItem["Assets", 300]);
	class AssetsMenu : public Reflect::Base
	{
		HORIZON_TYPE_REFLECT(AssetsMenu);
	};

	HCLASS(MainMenuItem["Build", 400]);
	class BuildMenu : public Reflect::Base
	{
		HORIZON_TYPE_REFLECT(BuildMenu);
	};

	HCLASS(MainMenuItem["Help", 500]);
	class HelpMenu : public Reflect::Base
	{
		HORIZON_TYPE_REFLECT(HelpMenu);
	};
}