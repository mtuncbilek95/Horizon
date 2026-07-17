#pragma once

#include <Editor/Widget/IWidget.h>
#include <Editor/Widget/DockLayout.h>
#include <Editor/Widget/WidgetAttribute.h>
#include <Editor/Font/IconsFontAwesome6.h>

#include <imgui.h>

#include "AssetBrowserWidget.reflected.h"

namespace Horizon
{
	class DomainFile;
	class DomainFolder;

	HCLASS(WidgetAttribute["Asset Browser", ICON_FA_DATABASE, DockLayout::Bottom, true])
	class AssetBrowserWidget : public IWidget
	{
		HORIZON_REFLECT;
	public:
		void OnInvoke() final;
		void OnDraw() final;

	private:
		void ActivateAsset(DomainFile* pFile);

	private:
		DomainFolder* m_currentFolder = nullptr;
		ImGuiSelectionBasicStorage m_selection;
	};
}