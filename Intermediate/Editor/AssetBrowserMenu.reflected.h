#pragma once

#include <Editor/Menu/Templates/Window/AssetBrowserMenu.h>

namespace Horizon::Reflect
{
	template<>
	struct TypeAccessor<Horizon::AssetBrowserMenu>
	{
		static Type Build()
		{
				return TypeBuilder<Horizon::AssetBrowserMenu>::ForType("AssetBrowserMenu")
					.WithBase<Horizon::IMenuItem>()
					.WithAttribute<Horizon::MenuItemAttribute>("Window/Panels/Asset Browser", 1u)
					.Build();
		}
	};
}
