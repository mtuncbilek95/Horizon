#pragma once

#include <Editor/ContextMenu/ContextMenuItemAttribute.h>
#include <Editor/ContextMenu/AssetBrowserMenu/AssetBrowserMenuItem.h>

namespace Horizon::Editor
{
	HCLASS(ContextMenuItem["ContentBrowserView", "Rename", 2]);
	class RenameObjectItem : public AssetBrowserMenuItem
	{
		HORIZON_TYPE_REFLECT(RenameObjectItem);
	public:
		void OnExecute(AssetBrowserContext& context) final;
		b8 IsEnabled(const AssetBrowserContext& context) final;
	};
}