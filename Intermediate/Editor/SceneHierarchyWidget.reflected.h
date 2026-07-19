#pragma once

#include <Editor/Widget/SceneHierarchy/SceneHierarchyWidget.h>

namespace Horizon::Reflect
{
	template<>
	struct TypeAccessor<Horizon::SceneHierarchyWidget>
	{
		static Type Build()
		{
				return TypeBuilder<Horizon::SceneHierarchyWidget>::ForType("SceneHierarchyWidget")
					.WithBase<Horizon::IWidget>()
					.WithAttribute<Horizon::WidgetTypeAttribute>("Scene Hierarchy", ICON_FA_SITEMAP, DockLayout::Left, true)
					.Build();
		}
	};
}
