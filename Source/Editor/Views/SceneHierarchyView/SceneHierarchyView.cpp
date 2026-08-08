#include "SceneHierarchyView.h"

#include <Engine/Core/Application.h>

namespace Horizon::Editor
{
	void SceneHierarchyView::OnInvoke()
	{
		m_contextMenu.BootstrapContext(m_engine, "SceneHierarchyView");
	}

	void SceneHierarchyView::OnRender()
	{
		SceneHierarchyContext context = {};
		m_contextMenu.RenderGUI("SceneHierarchyContextMenu", context);
	}
}