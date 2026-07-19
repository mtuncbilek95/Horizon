#pragma once

#include <Editor/Widget/AssetBrowser/AssetBrowserWidget.h>

namespace Horizon::Reflect
{
	template<>
	struct TypeAccessor<Horizon::AssetBrowserWidget>
	{
		static Type Build()
		{
				return TypeBuilder<Horizon::AssetBrowserWidget>::ForType("AssetBrowserWidget")
					.WithBase<Horizon::IWidget>()
					.WithAttribute<Horizon::WidgetTypeAttribute>("Asset Browser", ICON_FA_DATABASE, DockLayout::Bottom, true)
					.Build();
		}
	};
}
