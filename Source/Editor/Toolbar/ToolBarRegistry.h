#pragma once

#include <Editor/Renderer/EditorContext.h>
#include <Editor/ToolBar/ToolBarItem.h>
#include <Editor/ToolBar/ToolBarSection.h>
#include <Runtime/Containers/List.h>

namespace Horizon::Editor
{
	class H_EXPORT ToolBarRegistry
	{
		static constexpr f32 kHeight = 36.0f;
		static constexpr usize kSectionCount = 3;

		struct ItemEntry
		{
			ToolBarItem* pItem = nullptr;
			i32 order = 0;
		};
	public:
		ToolBarRegistry() = default;
		~ToolBarRegistry();

		void BootstrapItems(const EditorContext& ctx);
		void RenderGUI();

	private:
		void RenderSection(ToolBarSection section, f32 cursorX);
		void Clear();

	private:
		EditorContext m_context;
		List<ItemEntry> m_sections[kSectionCount];
		f32 m_sectionWidths[kSectionCount] = {};
	};
}