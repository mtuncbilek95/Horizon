#pragma once

#include <Editor/Menu/IMenuItem.h>
#include <Editor/Menu/MenuAttribute.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon
{
	HCLASS(MenuItemAttribute["File/Exit", 100u]);
	class ExitEngineMenu : public IMenuItem
	{
		HORIZON_TYPE_REFLECT;

	public:
		void OnInvoke() final;
	};
}