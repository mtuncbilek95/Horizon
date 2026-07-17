#pragma once

#include <Runtime/Reflection/Reflect.h>
#include <Runtime/Reflection/FieldManifest.h>
#include <Runtime/Reflection/TypeAttribute.h>
#include <Runtime/Reflection/TypeManifest.h>
#include <Runtime/Reflection/TypeManifestBuilder.h>

namespace Horizon
{
	class EditorMainMenu;

	template<>
	struct Reflector<EditorMainMenu>
	{
		static TypeManifest Build()
		{
			TypeManifestBuilder builder;

			return builder.For<EditorMainMenu>("EditorMainMenu")
				.WithAttribute<MainMenuAttribute>("File", 10u)
				.WithAttribute<MainMenuAttribute>("Edit", 20u)
				.WithAttribute<MainMenuAttribute>("View", 40u)
				.WithAttribute<MainMenuAttribute>("View/Widgets", 1u)
				.Build();
		}
	};
}