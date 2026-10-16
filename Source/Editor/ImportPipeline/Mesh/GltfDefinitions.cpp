#include "GltfDefinitions.h"

#include <Runtime/Definitions/Allocator.h>

#include <fastgltf/core.hpp>

namespace Horizon::Editor
{
	void gltfParserDeleter::operator()(fastgltf::Parser* pParser) const
	{
		Memory::Allocator::Delete(pParser);
	}

	void gltfAssetDeleter::operator()(fastgltf::Asset* pAsset) const
	{
		Memory::Allocator::Delete(pAsset);
	}
}