#pragma once

#include <Editor/Views/ViewDescriptor.h>
#include <Editor/Views/ViewObject.h>

#include <Runtime/Containers/List.h>

namespace Horizon::Editor
{
	class H_EXPORT ViewRegistry
	{
	public:
		ViewRegistry(Engine::Engine* pEngine);
		~ViewRegistry();

		void BootstrapViews();
		void RenderGUI();

		template<typename T>
		T* GetViewObject()
		{
			return (T*)GetViewObject(Reflect::TypeOf<T>());
		}

		ViewObject* GetViewObject(Reflect::TypeHandle handl);

	private:
		void BuildDefaultLayout(u32 rootId);

	private:
		Engine::Engine* m_engine = nullptr;

		List<ViewDescriptor> m_registeredViews;
		List<ViewObject*> m_createdViews;

		b8 m_layoutDirty = false;
	};
}