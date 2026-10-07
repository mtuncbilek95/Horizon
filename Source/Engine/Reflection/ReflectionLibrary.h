#pragma once

#include <Runtime/Containers/List.h>
#include <Runtime/PAL/Module/SymbolLibrary.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	struct ReflectionLibrary
	{
		const PAL::SymbolLibrary* library = nullptr;
		List<Reflect::Type> types;
	};
}