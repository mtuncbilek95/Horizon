#pragma once

#include <Runtime/Reflection/Reflect.h>
#include <Runtime/Reflection/FieldManifest.h>
#include <Runtime/Reflection/TypeAttribute.h>
#include <Runtime/Reflection/TypeManifest.h>
#include <Runtime/Reflection/TypeManifestBuilder.h>

namespace Horizon
{
	class ExitEngineMenu;

	template<>
	struct Reflector<ExitEngineMenu>
	{
		static TypeManifest Build()
		{
			TypeManifestBuilder builder;

			return builder.For<ExitEngineMenu>("ExitEngineMenu")
				.WithBase(TypeIdOf<IMenuItem>())
				.WithAttribute<MenuItemAttribute>("File/Exit", 100u)
				.Build();
		}
	};
}