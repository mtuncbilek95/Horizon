#pragma once

#include <Editor/Renderer/EditorContext.h>
#include <Editor/Views/ViewDescriptor.h>
#include <Editor/Views/ViewObject.h>
#include <Engine/Reflection/ReflectionLibrary.h>

#include <Runtime/Containers/List.h>

namespace Horizon::Editor
{
	class EDITOR_API ViewRegistry
	{
	public:
		ViewRegistry();
		~ViewRegistry();

		void BootstrapViews(const EditorContext& ctx);
		void RenderGUI(const Engine::EngineFrame& context);

		void OnLibraryRegistered(const Engine::ReflectionLibrary& library);
		void OnLibraryUnregistered(const Engine::ReflectionLibrary& library);

		template<typename T>
		T* GetViewObject()
		{
			return (T*)GetViewObject(Reflect::TypeOf<T>());
		}

		template<typename T>
		T* OpenView()
		{
			return (T*)OpenView(Reflect::TypeOf<T>());
		}

		b8 HasViewObject(Reflect::TypeHandle handl);
		ViewObject* GetViewObject(Reflect::TypeHandle handl);
		ViewObject* OpenView(Reflect::TypeHandle handl);
		void CloseView(ViewObject* pView);

		const List<ViewDescriptor>& GetDescriptors() const { return m_registeredViews; }

	private:
		b8 AddType(const Reflect::Type* pType);
		const ViewDescriptor* FindDescriptor(Reflect::TypeHandle handl) const;
		ViewObject* CreateView(const ViewDescriptor& descriptor);
		void BuildDefaultLayout(u32 rootId);

	private:
		EditorContext m_context;

		List<ViewDescriptor> m_registeredViews;
		List<ViewObject*> m_createdViews;

		u32 m_instanceCounter = 0;
		b8 m_layoutDirty = false;
	};
}