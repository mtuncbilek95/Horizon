#pragma once

#include <Editor/Font/IconsFontAwesome6.h>

#include <Runtime/Reflection/Reflect.h>
#include <Runtime/Reflection/FieldManifest.h>
#include <Runtime/Reflection/TypeAttribute.h>
#include <Runtime/Reflection/TypeManifest.h>
#include <Runtime/Reflection/TypeManifestBuilder.h>

namespace Horizon
{
	class SceneHierarchyWidget;

	template<>
	struct Reflector<SceneHierarchyWidget>
	{
		static TypeManifest Build()
		{
			TypeManifestBuilder builder;

			return builder.For<SceneHierarchyWidget>("SceneHierarchyWidget")
				.WithBase(TypeIdOf<IWidget>())
				.WithAttribute<WidgetTypeAttribute>(
					"Scene Hierarchy", ICON_FA_SITEMAP, DockLayout::Left, true)
				.Build();
		}
	};
}