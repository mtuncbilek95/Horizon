#pragma once

#include <memory>

namespace fastgltf
{
	class Parser;
	class Asset;
}

namespace Horizon::Editor
{
	struct gltfParserDeleter
	{
		void operator()(fastgltf::Parser* pParser) const;
	};

	struct gltfAssetDeleter
	{
		void operator()(fastgltf::Asset* pAsset) const;
	};

	using gltfParserPtr = std::unique_ptr<fastgltf::Parser, gltfParserDeleter>;
	using gltfAssetPtr = std::unique_ptr<fastgltf::Asset, gltfAssetDeleter>;
}