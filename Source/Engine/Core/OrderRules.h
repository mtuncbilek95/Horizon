#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

#include <vector>
#include <typeindex>

namespace Horizon
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
		std::vector<std::type_index> after;
		std::vector<std::type_index> before;
	};
}