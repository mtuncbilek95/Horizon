#pragma once

#include <Runtime/Reflection/Reflect.h>
#include <Runtime/Reflection/FieldManifest.h>
#include <Runtime/Reflection/TypeAttribute.h>
#include <Runtime/Reflection/TypeManifest.h>
#include <Runtime/Reflection/TypeManifestBuilder.h>

namespace Horizon
{
	class SceneHierarchyMenu;

	template<>
	struct Reflector<SceneHierarchyMenu>
	{
		static TypeManifest Build()
		{
			TypeManifestBuilder builder;

			return builder.For<SceneHierarchyMenu>("SceneHierarchyMenu")
				.WithBase(TypeIdOf<IMenuItem>())
				.WithAttribute<MenuItemAttribute>("View/Widgets/Scene Hierarchy", 2u)
				.Build();
		}
	};
}