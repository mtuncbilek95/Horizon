#pragma once

#include <Editor/Font/IconsFontAwesome6.h>
#include <Editor/Widget/IWidget.h>
#include <Editor/Widget/WidgetAttribute.h>
#include <Runtime/RTTR/Reflection.h>

#include <imgui.h>

namespace Horizon
{
	class DomainFolder;
	class AssetBrowserMenuRegistry;

	HCLASS(WidgetTypeAttribute["Asset Browser", ICON_FA_DATABASE, DockLayout::Bottom, true]);
	class AssetBrowserWidget : public IWidget
	{
		HORIZON_TYPE_REFLECT;
	public:
		void OnInvoke() final;
		void OnDraw() final;

	private:
		AssetBrowserMenuRegistry* m_menuRegistry = nullptr;
		DomainFolder* m_currentFolder = nullptr;

		ImGuiSelectionBasicStorage m_selection;
	};
}