#pragma once

#include <string>

namespace Horizon
{
	struct H_EXPORT ImportDescriptor final
	{
		std::string fileName = "DefaultFile";
		std::string fileExtension;
	};
}