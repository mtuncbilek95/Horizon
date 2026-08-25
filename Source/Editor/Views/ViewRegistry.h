#pragma once

#include <Editor/Renderer/EditorContext.h>
#include <Editor/Views/ViewDescriptor.h>
#include <Editor/Views/ViewObject.h>

#include <Runtime/Containers/List.h>

namespace Horizon::Editor
{
	class H_EXPORT ViewRegistry
	{
	public:
		ViewRegistry();
		~ViewRegistry();

		void BootstrapViews(const EditorContext& ctx);
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
		EditorContext m_context;

		List<ViewDescriptor> m_registeredViews;
		List<ViewObject*> m_createdViews;

		b8 m_layoutDirty = false;
	};
}