#pragma once

#include <Runtime/Containers/Guid.h>
#include <string>

namespace Horizon
{
	struct H_EXPORT DomainAsset
	{
		std::string subName;
		std::string assetTypeName;
		Guid guid;
	};
}