#pragma once

#include <Editor/Widget/IWidget.h>

#include <imgui.h>

namespace Horizon
{
	class DomainFolder;

	class AssetBrowserWidget : public IWidget
	{
	public:
		void OnInvoke() final;
		void OnDraw() final;

	private:
		DomainFolder* m_currentFolder = nullptr;
		ImGuiSelectionBasicStorage m_selection;
	};
}