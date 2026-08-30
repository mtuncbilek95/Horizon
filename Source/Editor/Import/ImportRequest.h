#pragma once

#include <Runtime/Containers/Guid.h>
#include <string>

namespace Horizon::Editor
{
	struct ImportRequest
	{
		Guid guid;
		std::string sourcePath;
		std::string extension;
	};
}