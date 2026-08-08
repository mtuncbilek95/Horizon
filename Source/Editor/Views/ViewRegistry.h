#pragma once

#include <Editor/Views/ViewDescriptor.h>
#include <Editor/Views/ViewObject.h>

#include <Runtime/Containers/List.h>

namespace Horizon::Editor
{
	class H_EXPORT ViewRegistry
	{
	public:
		ViewRegistry(Engine::Application* pEngine);
		~ViewRegistry();

		void BootstrapViews();
		void RenderGUI();

	private:
		void BuildDefaultLayout(u32 rootId);

	private:
		Engine::Application* m_engine = nullptr;

		List<ViewDescriptor> m_registeredViews;
		List<ViewObject*> m_createdViews;

		b8 m_layoutDirty = false;
	};
}