#pragma once

#include <Runtime/Reflection/Reflect.h>
#include <Runtime/Reflection/FieldManifest.h>
#include <Runtime/Reflection/TypeAttribute.h>
#include <Runtime/Reflection/TypeManifest.h>
#include <Runtime/Reflection/TypeManifestBuilder.h>

namespace Horizon
{
	class AssetBrowserMenu;

	template<>
	struct Reflector<AssetBrowserMenu>
	{
		static TypeManifest Build()
		{
			TypeManifestBuilder builder;

			return builder.For<AssetBrowserMenu>("AssetBrowserMenu")
				.WithBase(TypeIdOf<IMenuItem>())
				.WithAttribute<MenuItemAttribute>("View/Widgets/Asset Browser", 1u)
				.Build();
		}
	};
}