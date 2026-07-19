#pragma once

#include <Editor/Font/IconsFontAwesome6.h>
#include <Editor/Widget/IWidget.h>
#include <Editor/Widget/WidgetAttribute.h>
#include <Runtime/RTTR/Reflection.h>

#include <imgui.h>

namespace Horizon
{
	class DomainFile;

	HCLASS(WidgetTypeAttribute["Scene Hierarchy", ICON_FA_SITEMAP, DockLayout::Left, true]);
	class SceneHierarchyWidget : public IWidget
	{
		HORIZON_TYPE_REFLECT;
	public:
		void OnInvoke() final;
		void OnDraw() final;

	private:
		DomainFile* m_currentFolder = nullptr;
	};
}