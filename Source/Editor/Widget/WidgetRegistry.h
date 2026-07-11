#pragma once

#include <Editor/Widget/WidgetFactory.h>

#include <vector>

namespace Horizon
{
	class Engine;
	class IWidget;

	class WidgetRegistry
	{
		struct WidgetInstance
		{
			IWidget* widget;
			std::string title;
			WidgetDock dock;
			b8 isOpen;
			std::type_index type;
		};
	public:
		WidgetRegistry(Engine* pEngine);
		~WidgetRegistry();

		void Invalidate();
		void Render();

		void Open(const WidgetTypeInfo& info);
		void Open(const std::type_index& index);
		void Close(const std::type_index& index);

		b8 IsOpened(const std::type_index& index);

	private:
		void BuildDefaultLayout(u32 rootId);

	private:
		Engine* m_engine;
		std::vector<WidgetInstance> m_widgets;
		std::unordered_map<std::type_index, usize> m_lookup;

		b8 m_layout = false;
	};
}