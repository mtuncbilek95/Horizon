#pragma once

#include <Editor/Attributes/ContextMenuItemAttribute.h>
#include <Editor/ContextMenu/AssetBrowserMenu/AssetBrowserMenuItem.h>

namespace Horizon::Editor
{
	HCLASS(ContextMenuItem["AssetBrowserView", "Create/Folder", 0]);
	class H_EXPORT CreateFolderItem : public AssetBrowserMenuItem
	{
	public:
		void OnExecute(const AssetBrowserContext& context) final;
		b8 IsEnabled(const AssetBrowserContext& context) final;
	};
}