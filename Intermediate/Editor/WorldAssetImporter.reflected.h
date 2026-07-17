#pragma once

#include <Runtime/Reflection/Reflect.h>
#include <Runtime/Reflection/FieldManifest.h>
#include <Runtime/Reflection/TypeAttribute.h>
#include <Runtime/Reflection/TypeManifest.h>
#include <Runtime/Reflection/TypeManifestBuilder.h>

namespace Horizon
{
	class WorldAssetImporter;

	template<>
	struct Reflector<WorldAssetImporter>
	{
		static TypeManifest Build()
		{
			TypeManifestBuilder builder;

			return builder.For<WorldAssetImporter>("WorldAssetImporter")
				.WithBase(TypeIdOf<IAssetImporter>())
				.WithAttribute<AssetImporterAttribute>(
					std::vector<std::string_view>{ ".hworld" }, "DefaultScene", 1u)
				.Build();
		}
	};
}