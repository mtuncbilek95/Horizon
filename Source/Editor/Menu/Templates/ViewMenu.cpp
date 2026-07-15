#include "ViewMenu.h"

#include <Engine/Core/Engine.h>

#include <Editor/Renderer/EditorSystem.h>
#include <Editor/Menu/MenuDefinitions.h>

#include <Editor/Widget/WidgetRegistry.h>
#include <Editor/Widget/AssetBrowser/AssetBrowserWidget.h>
#include <Editor/Widget/SceneHierarchy/SceneHierarchyWidget.h>

namespace Horizon
{
	void AssetBrowserMenu::OnInvoke()
	{
		auto* editorSub = GetEngine()->TryGetSystem<EditorSystem>();
		auto* widgetReg = editorSub->GetWidgetRegistry();

		/*std::type_index type = typeid(AssetBrowserWidget);
		if(widgetReg->IsOpened(type))
			widgetReg->Close(type);
		else
			widgetReg->Open(type);*/
	}

	void SceneHierarchyMenu::OnInvoke()
	{
		auto* editorSub = GetEngine()->TryGetSystem<EditorSystem>();
		auto* widgetReg = editorSub->GetWidgetRegistry();

		/*std::type_index type = typeid(SceneHierarchyWidget);
		if (widgetReg->IsOpened(type))
			widgetReg->Close(type);
		else
			widgetReg->Open(type);*/
	}
}

HORIZON_DEFINE_MENU("View/Widgets", 1);

HORIZON_BIND_MENU(AssetBrowserMenu, "View/Widgets/Asset Browser", 1);
HORIZON_BIND_MENU(SceneHierarchyMenu, "View/Widgets/Scene Hierarchy", 2);