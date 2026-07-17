#pragma once

#include <Editor/Menu/IMenuItem.h>
#include <Editor/Menu/MenuAttribute.h>
#include "SceneHierarchyMenu.reflected.h"

namespace Horizon
{
	HCLASS(MenuItemAttribute["View/Widgets/Scene Hierarchy", 2u]);
	class SceneHierarchyMenu : public IMenuItem
	{
		HORIZON_REFLECT;
	public:
		void OnInvoke() final;
	};
}