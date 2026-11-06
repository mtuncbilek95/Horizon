#pragma once

#include <Editor/Renderer/EditorContext.h>
#include <Editor/ToolBar/ToolBarItem.h>
#include <Editor/ToolBar/ToolBarSection.h>
#include <Runtime/RTTR/Reflection.h>
#include <Runtime/Containers/Vector/List.h>

namespace Horizon::Editor
{
	class ToolBarRegistry
	{
		static constexpr f32 kHeight = 36.0f;
		static constexpr usize kSectionCount = 3;

		struct ItemEntry
		{
			ToolBarItem* pItem = nullptr;
			const Reflect::Type* pType = nullptr;
			i32 order = 0;
		};

	public:
		ToolBarRegistry() = default;
		~ToolBarRegistry();

		void BootstrapItems(EditorContext* ctx);
		void RenderGUI();

	private:
		b8 AddType(const Reflect::Type* pType);
		void RemoveType(const Reflect::Type* pType);
		void SortSections();

		void RenderSection(ToolBarSection section, f32 cursorX);
		void Clear();

	private:
		EditorContext* m_context;
		List<ItemEntry> m_sections[kSectionCount];
		f32 m_sectionWidths[kSectionCount] = {};
	};
}