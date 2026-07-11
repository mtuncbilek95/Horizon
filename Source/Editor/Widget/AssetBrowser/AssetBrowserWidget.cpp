#include "AssetBrowserWidget.h"

#include <Engine/Core/Engine.h>

#include <Editor/Domain/DomainSystem.h>
#include <Editor/Widget/WidgetDefinitions.h>
#include <Editor/Font/IconsFontAwesome6.h>

#include <imgui.h>

namespace Horizon
{
	void AssetBrowserWidget::OnInvoke()
	{
		auto* domainSub = GetEngine()->TryGetSystem<DomainSystem>();
		
	}

	void AssetBrowserWidget::OnDraw()
	{
	}
}

HORIZON_BIND_WIDGET(AssetBrowserWidget, "Asset Browser", ICON_FA_DATABASE, WidgetDock::Bottom, true);