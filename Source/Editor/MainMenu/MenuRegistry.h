#pragma once

#include <Editor/MainMenu/MenuItemInstance.h>
#include <Editor/Renderer/EditorContext.h>
#include <Engine/Reflection/ReflectionLibrary.h>

namespace Horizon::Engine
{
	class Engine;
}

namespace Horizon::Editor
{
	class MenuRegistry
	{
	public:
		MenuRegistry() = default;
		~MenuRegistry();

		void BootstrapMenus(EditorContext* ctx);
		void RenderGUI();

		void OnLibraryRegistered(const Engine::ReflectionLibrary& library);
		void OnLibraryUnregistered(const Engine::ReflectionLibrary& library);

	private:
		void AddRootType(const Reflect::Type* pType);
		void AddLeafType(const Reflect::Type* pType);
		b8 RemoveType(List<MenuItemInstance>& siblings, const Reflect::Type* pType);

		void RenderNode(MenuItemInstance& inst);
		void ClearRecursive(MenuItemInstance& inst);
		void SortRecursive(List<MenuItemInstance>& siblings);

		MenuItemInstance& FindOrCreateContainer(List<MenuItemInstance>& siblings, const std::string& name);

	private:
		EditorContext* m_context = nullptr;

		PAL::Window* m_window = nullptr;
		List<MenuItemInstance> m_menus;
	};
}