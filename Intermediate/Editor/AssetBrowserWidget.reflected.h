#pragma once

#include <Runtime/Reflection/Reflect.h>
#include <Runtime/Reflection/FieldManifest.h>
#include <Runtime/Reflection/TypeAttribute.h>
#include <Runtime/Reflection/TypeManifest.h>
#include <Runtime/Reflection/TypeManifestBuilder.h>

namespace Horizon
{
	class AssetBrowserWidget;

	template<>
	struct Reflector<AssetBrowserWidget>
	{
		static TypeManifest Build()
		{
			TypeManifestBuilder builder;

			return builder.For<AssetBrowserWidget>("AssetBrowserWidget")
				.WithBase(TypeIdOf<IWidget>())
				.WithAttribute<WidgetTypeAttribute>(
					"Asset Browser", ICON_FA_DATABASE, DockLayout::Bottom, true)
				.Build();
		}
	};
}