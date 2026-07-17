#pragma once

#include <Runtime/Reflection/Reflect.h>
#include <Runtime/Reflection/FieldManifest.h>
#include <Runtime/Reflection/TypeAttribute.h>
#include <Runtime/Reflection/TypeManifest.h>
#include <Runtime/Reflection/TypeManifestBuilder.h>

namespace Horizon
{
	class DefaultWorldLoadStrategy;

	template<>
	struct Reflector<DefaultWorldLoadStrategy>
	{
		static TypeManifest Build()
		{
			TypeManifestBuilder builder;

			return builder.For<DefaultWorldLoadStrategy>("DefaultWorldLoadStrategy")
				.WithBase(TypeIdOf<IAssetLoadStrategy>())
				.WithAttribute<AssetLoadStrategyAttribute>("World", true)
				.Build();
		}
	};
}