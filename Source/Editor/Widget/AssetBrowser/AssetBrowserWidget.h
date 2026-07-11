#pragma once

#include <Editor/Widget/IWidget.h>

namespace Horizon
{
	class AssetBrowserWidget : public IWidget
	{
	public:
		void OnInvoke() final;
		void OnDraw() final;
	};
}