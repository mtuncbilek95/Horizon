#include "SceneHierarchyMenu.h"

#include <Editor/Renderer/EditorSystem.h>
#include <Editor/Widget/WidgetRegistry.h>
#include <Editor/Widget/SceneHierarchy/SceneHierarchyWidget.h>
#include <Engine/Core/Engine.h>

namespace Horizon
{
	void SceneHierarchyMenu::OnInvoke()
	{
		auto* editorSub = GetEngine()->TryGetSystem<EditorSystem>();
		if (!editorSub)
			return;

		auto* widgetReg = editorSub->GetWidgetRegistry();
		if (!widgetReg)
			return;

		widgetReg->Toggle(TypeIdOf<SceneHierarchyWidget>());
	}
}