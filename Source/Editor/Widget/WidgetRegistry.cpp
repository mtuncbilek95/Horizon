#include "WidgetRegistry.h"

#include <Editor/Widget/IWidget.h>

namespace Horizon
{
	WidgetRegistry::WidgetRegistry(Engine* pEngine) : m_engine(pEngine)
	{
	}

	WidgetRegistry::~WidgetRegistry()
	{
		for (auto* widget : m_widgets)
			Allocator::Delete(widget);
	}

	void WidgetRegistry::Invalidate()
	{
		for (auto* widget : m_widgets)
			Allocator::Delete(widget);

		m_widgets.clear();

	}

	void WidgetRegistry::Render()
	{
	}
}