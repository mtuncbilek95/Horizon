#include "GltfDefinitions.h"


#include <fastgltf/core.hpp>

namespace Horizon::Editor
{
	void gltfParserDeleter::operator()(fastgltf::Parser* pParser) const
	{
		delete pParser;
	}

	void gltfAssetDeleter::operator()(fastgltf::Asset* pAsset) const
	{
		delete pAsset;
	}
}