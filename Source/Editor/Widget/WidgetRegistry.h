#pragma once

#include <Editor/Widget/DockLayout.h>
#include <Runtime/Reflection/TypeManifest.h>

#include <vector>
#include <string>
#include <string_view>

namespace Horizon
{
	class Engine;
	class IWidget;

	class WidgetRegistry
	{
		struct WidgetType
		{
			TypeManifest* widgetManifest;
			std::string widgetDisplayName;
			DockLayout dock;
			b8 alwaysOpenFirst = false;
		};

		struct OpenWidget
		{
			IWidget* widget;
			TypeManifest* manifest;
			std::string title;
			b8 open = true;
		};
	public:
		WidgetRegistry(Engine* pEngine);
		~WidgetRegistry();

		void Invalidate();
		void Render();

		void Open(ReflectionTypeHandle typeId);
		void Close(ReflectionTypeHandle typeId);
		void Toggle(ReflectionTypeHandle typeId);
		b8 IsOpened(ReflectionTypeHandle typeId) const;

	private:
		void Open(const WidgetType& type);
		void BuildDefaultLayout(u32 rootId);

	private:
		Engine* m_engine;
		std::vector<WidgetType> m_types;
		std::vector<OpenWidget> m_open;

		b8 m_layout = false;
	};
}