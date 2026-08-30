#pragma once

#include <Editor/Attributes/ContextMenuItemAttribute.h>
#include <Editor/ContextMenu/AssetBrowserMenu/AssetBrowserMenuItem.h>

namespace Horizon::Editor
{
	HCLASS(ContextMenuItem["AssetBrowserView", "Create/Scene", 1]);
	class H_EXPORT CreateSceneItem : public AssetBrowserMenuItem
	{
		HORIZON_TYPE_REFLECT(CreateSceneItem);
	public:
		void OnExecute(AssetBrowserContext& context) final;
		b8 IsEnabled(const AssetBrowserContext& context) final;
	};
}