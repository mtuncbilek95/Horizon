#pragma once

#include <Editor/ContextMenu/AssetBrowser/AssetBrowserMenuAttribute.h>
#include <Editor/ContextMenu/AssetBrowser/AssetBrowserMenuItem.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon
{
	HCLASS(AssetBrowserMenuItemAttribute["Create/Folder", 1u]);
	class CreateFolderAssetBrowserMenu : public AssetBrowserMenuItem
	{
		HORIZON_TYPE_REFLECT;

	public:
		void OnInvoke(const AssetBrowserContext& context) final;
	};
}