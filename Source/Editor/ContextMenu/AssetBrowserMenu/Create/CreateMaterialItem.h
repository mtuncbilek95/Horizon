#pragma once

#include <Editor/ContextMenu/ContextMenuItemAttribute.h>
#include <Editor/ContextMenu/AssetBrowserMenu/AssetBrowserMenuItem.h>

namespace Horizon::Editor
{
	HCLASS(ContextMenuItem["ContentBrowserView", "Create/Material", 2]);
	class EDITOR_API CreateMaterialItem : public AssetBrowserMenuItem
	{
		HORIZON_TYPE_REFLECT(CreateMaterialItem);
	public:
		void OnExecute(AssetBrowserContext& context) final;
		b8 IsEnabled(const AssetBrowserContext& context) final;
	};
}