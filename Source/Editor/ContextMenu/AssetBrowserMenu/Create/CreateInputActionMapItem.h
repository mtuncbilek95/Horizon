#pragma once

#include <Editor/ContextMenu/ContextMenuItemAttribute.h>
#include <Editor/ContextMenu/AssetBrowserMenu/AssetBrowserMenuItem.h>

namespace Horizon::Editor
{
	HCLASS(ContextMenuItem["ContentBrowserView", "Create/Input Action Map", 3]);
	class CreateInputActionMapItem : public AssetBrowserMenuItem
	{
		HORIZON_TYPE_REFLECT(CreateInputActionMapItem);
	public:
		void OnExecute(AssetBrowserContext& context) final;
		b8 IsEnabled(const AssetBrowserContext& context) final;
	};
}
