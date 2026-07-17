#include "AssetBrowserMenu.h"

#include <Editor/Renderer/EditorSystem.h>
#include <Editor/Widget/WidgetRegistry.h>
#include <Editor/Widget/AssetBrowser/AssetBrowserWidget.h>
#include <Engine/Core/Engine.h>

namespace Horizon
{
	void AssetBrowserMenu::OnInvoke()
	{
		auto* editorSub = GetEngine()->TryGetSystem<EditorSystem>();
		if (!editorSub)
			return;

		auto* widgetReg = editorSub->GetWidgetRegistry();
		if (!widgetReg)
			return;

		widgetReg->Toggle(TypeIdOf<AssetBrowserWidget>());
	}
}