#pragma once

#include <Editor/Attributes/ContextMenuItemAttribute.h>
#include <Editor/ContextMenu/AssetBrowserMenu/AssetBrowserMenuItem.h>

namespace Horizon::Editor
{
	HCLASS(ContextMenuItem["AssetBrowserView", "Delete", 1]);
	class H_EXPORT DeleteObjectItem : public AssetBrowserMenuItem
	{
	public:
		void OnExecute(AssetBrowserContext& context) final;
		b8 IsEnabled(const AssetBrowserContext& context) final;
	};
}