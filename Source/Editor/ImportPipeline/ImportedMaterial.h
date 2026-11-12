#pragma once

#include <Engine/Asset/Material/MaterialParameters.h>
#include <Engine/Asset/Material/MaterialProperties.h>

#include <string>

namespace Horizon::Editor
{
	struct ImportedMaterial
	{
		std::string name;
		Asset::MaterialProperties properties;
		Asset::MaterialParameters parameters;

		std::string baseColorPath;
		std::string normalPath;
		std::string occlusionPath;
		std::string roughnessPath;
		std::string metallicPath;
		std::string emissivePath;
	};
}
