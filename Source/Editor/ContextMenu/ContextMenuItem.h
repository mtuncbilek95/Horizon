#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::Editor
{
	template<typename TCtx>
	class ContextMenuItem
	{
	public:
		virtual ~ContextMenuItem() = default;

		virtual void OnExecute(TCtx& context) = 0;
		virtual b8 IsEnabled(const TCtx& context) = 0;
	};
}