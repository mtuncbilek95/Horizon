#pragma once

#include <Editor/Attributes/ContextMenuItemAttribute.h>
#include <Editor/ContextMenu/AssetBrowserMenu/AssetBrowserMenuItem.h>

namespace Horizon
{
	HCLASS(ContextMenuItem["AssetBrowser/Rename", 100]);
	class H_EXPORT RenameAssetItem : public AssetBrowserMenuItem
	{
	public:
		void OnExecute(AssetBrowserMenuContext& context) final;
		b8 IsEnabled(const AssetBrowserMenuContext& context) const final;
	};
}