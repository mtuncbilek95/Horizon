#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon
{
	template<typename TContext>
	class ContextMenuItem
	{
	public:
		virtual ~ContextMenuItem() = default;

		virtual void OnExecute(TContext& context) = 0;
		virtual b8 IsEnabled(const TContext& context) const { return true; }
	};
}