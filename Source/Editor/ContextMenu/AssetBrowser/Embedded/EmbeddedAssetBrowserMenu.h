#pragma once

#include <Editor/ContextMenu/AssetBrowser/AssetBrowserMenuAttribute.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon
{
	HCLASS();
	class EmbeddedAssetBrowserMenu
	{
		HATTRIBUTE(AssetBrowserMainAttribute["Create", 10u]);

		HATTRIBUTE(AssetBrowserMainAttribute["Create/Lights", 10u]);
		HATTRIBUTE(AssetBrowserMainAttribute["Create/2D Objects", 20u]);
		HATTRIBUTE(AssetBrowserMainAttribute["Create/3D Objects", 30u]);
	};
}