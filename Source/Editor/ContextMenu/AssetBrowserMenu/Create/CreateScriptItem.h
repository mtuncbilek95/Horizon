#pragma once

#include <Editor/ContextMenu/ContextMenuItemAttribute.h>
#include <Editor/ContextMenu/AssetBrowserMenu/AssetBrowserMenuItem.h>

namespace Horizon::Editor
{
	HCLASS(ContextMenuItem["AssetBrowserView", "Create/Script", 2]);
	class EDITOR_API CreateScriptItem : public AssetBrowserMenuItem
	{
		HORIZON_TYPE_REFLECT(CreateScriptItem);
	public:
		void OnExecute(AssetBrowserContext& context) final;
		b8 IsEnabled(const AssetBrowserContext& context) final;
	};
}