#pragma once

#include <Editor/Attributes/EditorViewAttribute.h>
#include <Editor/Views/ViewObject.h>
#include <Editor/Font/IconsFontAwesome6.h>

namespace Horizon::Editor
{
	HCLASS(EditorView[ICON_FA_FOLDER_TREE, "Asset Browser", false, true, DockZone::Bottom]);
	class H_EXPORT AssetBrowserView : public ViewObject
	{
	public:
		void OnInvoke() final;
		void OnRender() final;

	private:
	};
}