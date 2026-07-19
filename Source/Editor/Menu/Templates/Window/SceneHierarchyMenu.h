#pragma once

#include <Editor/Menu/IMenuItem.h>
#include <Editor/Menu/MenuAttribute.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon
{
	HCLASS(MenuItemAttribute["Window/Panels/Scene Hierarchy", 2u]);
	class SceneHierarchyMenu : public IMenuItem
	{
		HORIZON_TYPE_REFLECT;

	public:
		void OnInvoke() final;
	};
}