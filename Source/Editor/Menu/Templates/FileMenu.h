#pragma once

#include <Editor/Menu/IMenuItem.h>

namespace Horizon
{
	class ExitEngineMenu : public IMenuItem
	{
	public:
		void OnInvoke() final;
	};
}