#include "AssetBrowserView.h"

#include <Engine/Core/Application.h>

namespace Horizon::Editor
{
	void AssetBrowserView::OnInvoke()
	{
		m_contextMenu.BootstrapContext(m_engine, "AssetBrowserView");
	}

	void AssetBrowserView::OnRender()
	{
		AssetBrowserContext context = {};
		m_contextMenu.RenderGUI("AssetBrowserContextMenu", context);
	}
}