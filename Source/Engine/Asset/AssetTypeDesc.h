#pragma once

#include <Engine/Asset/AssetOrigin.h>

#include <Runtime/Containers/List.h>
#include <Runtime/RTTR/Reflection.h>

#include <string>

namespace Horizon::Engine
{
	struct AssetTypeDesc
	{
		Reflect::Type* pType = nullptr;
		std::string typeName;
		u32 version = 0;
		AssetOrigin origin = AssetOrigin::Native;
		List<std::string> extensions;
	};
}