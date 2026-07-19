#pragma once

#include <string>

namespace Horizon
{
	struct ImportDescriptor final
	{
		std::string fileName = "DefaultFile";
		std::string fileExtension;
	};
}