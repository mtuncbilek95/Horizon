#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/Containers/List.h>

#include <typeindex>

namespace Horizon::Engine
{
	enum class OrderTier : u8
	{
		First,
		Default,
		Last
	};

	struct OrderRules
	{
		OrderTier tier = OrderTier::Default;
		List<std::type_index> after;
		List<std::type_index> before;
	};
}