#pragma once

#include <Editor/Menu/Templates/File/ExitEngineMenu.h>

namespace Horizon::Reflect
{
	template<>
	struct TypeAccessor<Horizon::ExitEngineMenu>
	{
		static Type Build()
		{
				return TypeBuilder<Horizon::ExitEngineMenu>::ForType("ExitEngineMenu")
					.WithBase<Horizon::IMenuItem>()
					.WithAttribute<Horizon::MenuItemAttribute>("File/Exit", 100u)
					.Build();
		}
	};
}
