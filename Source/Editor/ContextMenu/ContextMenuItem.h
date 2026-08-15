#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Editor
{
	template<typename TCtx>
	class ContextMenuItem : public Reflect::Base
	{
	public:
		virtual ~ContextMenuItem() = default;

		virtual void OnExecute(TCtx& context) = 0;
		virtual b8 IsEnabled(const TCtx& context) = 0;
	};
}