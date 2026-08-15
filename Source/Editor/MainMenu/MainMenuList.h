#pragma once

#include <Editor/Attributes/MainMenuItemAttribute.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Editor
{
	HCLASS(MainMenuItem["File", 0]);
	class H_EXPORT FileMenu : public Reflect::Base
	{
		HORIZON_TYPE_REFLECT(FileMenu);
	};

	HCLASS(MainMenuItem["Edit", 100]);
	class H_EXPORT EditMenu : public Reflect::Base
	{
		HORIZON_TYPE_REFLECT(EditMenu);
	};

	HCLASS(MainMenuItem["View", 200]);
	class H_EXPORT ViewMenu : public Reflect::Base
	{
		HORIZON_TYPE_REFLECT(ViewMenu);
	};

	HCLASS(MainMenuItem["Assets", 300]);
	class H_EXPORT AssetsMenu : public Reflect::Base
	{
		HORIZON_TYPE_REFLECT(AssetsMenu);
	};

	HCLASS(MainMenuItem["Build", 400]);
	class H_EXPORT BuildMenu : public Reflect::Base
	{
		HORIZON_TYPE_REFLECT(BuildMenu);
	};

	HCLASS(MainMenuItem["Help", 500]);
	class H_EXPORT HelpMenu : public Reflect::Base
	{
		HORIZON_TYPE_REFLECT(HelpMenu);
	};
}