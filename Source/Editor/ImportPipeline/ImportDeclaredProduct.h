#pragma once

#include <Runtime/RTTR/Reflection.h>

#include <string>

namespace Horizon::Editor
{
	struct ImportDeclaredProduct
	{
		std::string key;
		Reflect::TypeHandle assetType = {};
	};
}