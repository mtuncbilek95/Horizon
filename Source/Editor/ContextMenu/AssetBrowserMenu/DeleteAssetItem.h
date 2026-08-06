#pragma once

#include <Editor/Attributes/ContextMenuItemAttribute.h>
#include <Editor/ContextMenu/AssetBrowserMenu/AssetBrowserMenuItem.h>

namespace Horizon
{
	HCLASS(ContextMenuItem["AssetBrowser/Delete", 200]);
	class H_EXPORT DeleteAssetItem : public AssetBrowserMenuItem
	{
	public:
		void OnExecute(AssetBrowserMenuContext& context) final;
		b8 IsEnabled(const AssetBrowserMenuContext& context) const final;
	};
}