#pragma once

#include <Editor/Menu/IMenuItem.h>

namespace Horizon
{
	class AssetBrowserMenu : public IMenuItem
	{
	public:
		void OnInvoke() final;
	};

	class SceneHierarchyMenu : public IMenuItem
	{
	public:
		void OnInvoke() final;
	};
}