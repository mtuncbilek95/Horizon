#pragma once

#include <Editor/Menu/Templates/Window/SceneHierarchyMenu.h>

namespace Horizon::Reflect
{
	template<>
	struct TypeAccessor<Horizon::SceneHierarchyMenu>
	{
		static Type Build()
		{
				return TypeBuilder<Horizon::SceneHierarchyMenu>::ForType("SceneHierarchyMenu")
					.WithBase<Horizon::IMenuItem>()
					.WithAttribute<Horizon::MenuItemAttribute>("Window/Panels/Scene Hierarchy", 2u)
					.Build();
		}
	};
}
