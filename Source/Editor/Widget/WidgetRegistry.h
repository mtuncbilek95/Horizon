#pragma once

#include <vector>

namespace Horizon
{
	class Engine;
	class IWidget;

	class WidgetRegistry
	{
	public:
		WidgetRegistry(Engine* pEngine);
		~WidgetRegistry();

		void Invalidate();
		void Render();

	private:
		Engine* m_engine;

		std::vector<IWidget*> m_widgets;
	};
}