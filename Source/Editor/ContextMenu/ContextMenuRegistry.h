#pragma once

#include <Editor/ContextMenu/ContextMenuItemAttribute.h>
#include <Editor/ContextMenu/ContextMenuItem.h>
#include <Engine/Core/Engine.h>
#include <Engine/Reflection/ReflectionLibrary.h>
#include <Engine/Reflection/ReflectionSystem.h>

#include <imgui.h>

namespace Horizon::Editor
{
	template<typename TContext>
	class ContextMenuRegistry
	{
		struct ContextMenuNode
		{
			std::string displayName;
			i32 order = 0;

			ContextMenuItem<TContext>* pItem = nullptr;
			const Reflect::Type* pType = nullptr;
			List<ContextMenuNode> children;
		};
	public:
		~ContextMenuRegistry()
		{
			Clear();
		}

		void BootstrapContext(Engine::Engine* pEngine, const std::string& ownerId)
		{
			Clear();
			m_ownerId = ownerId;

			auto* pModule = pEngine->GetReflectionSystem();

			for (const Reflect::Type* pType : pModule->GetTypeByAttribute(Reflect::TypeOf<ContextMenuItemAttribute>()))
				AddType(pType);

			SortRecursive(m_menus);
		}

		void RenderGUI(std::string_view name, TContext& context)
		{
			if (ImGui::BeginPopupContextWindow(name.data(), ImGuiPopupFlags_MouseButtonRight))
			{
				for (auto& menu : m_menus)
					RenderMenuNode(menu, context);

				ImGui::EndPopup();
			}
		}

		void OnLibraryRegistered(const Engine::ReflectionLibrary& library)
		{
			for (const Reflect::Type& type : library.types)
				AddType(&type);

			SortRecursive(m_menus);
		}

		void OnLibraryUnregistered(const Engine::ReflectionLibrary& library)
		{
			for (const Reflect::Type& type : library.types)
				RemoveType(m_menus, &type);
		}

	private:
		void Clear()
		{
			for (auto& node : m_menus)
				ClearRecursive(node);

			m_menus.Clear();
		}

		b8 AddType(const Reflect::Type* pType)
		{
			auto* pAttr = pType->GetCustomAttribute<ContextMenuItemAttribute>();

			if (pAttr == nullptr)
				return false;

			if (pAttr->GetOwner() != m_ownerId)
				return false;

			InsertMenu(pType, pAttr);
			return true;
		}

		void InsertMenu(const Reflect::Type* pType, ContextMenuItemAttribute* pAttr)
		{
			const std::string& path = pAttr->GetPath();
			i32 order = pAttr->GetOrder();
			usize start = 0;

			List<ContextMenuNode>* pLevel = &m_menus;

			while (true)
			{
				usize slash = path.find('/', start);
				b8 isLeaf = (slash == std::string::npos);
				std::string segment = path.substr(start, isLeaf ? std::string::npos : slash - start);

				if (isLeaf)
				{
					ContextMenuNode& leaf = pLevel->EmplaceBack();
					leaf.displayName = std::move(segment);
					leaf.order = order;
					leaf.pType = pType;
					leaf.pItem = static_cast<ContextMenuItem<TContext>*>(pType->Create());
					break;
				}

				ContextMenuNode& container = FindOrCreateContainer(*pLevel, segment);

				if (order < container.order)
					container.order = order;

				pLevel = &container.children;
				start = slash + 1;
			}
		}

		b8 RemoveType(List<ContextMenuNode>& siblings, const Reflect::Type* pType)
		{
			b8 removed = false;

			for (usize i = siblings.GetCount(); i > 0; --i)
			{
				ContextMenuNode& node = siblings[i - 1];

				if (node.pType == pType)
				{
					ClearRecursive(node);
					siblings.RemoveAt(i - 1);
					removed = true;
					continue;
				}

				if (!RemoveType(node.children, pType))
					continue;

				removed = true;

				if (node.pItem == nullptr && node.children.IsEmpty())
					siblings.RemoveAt(i - 1);
			}

			return removed;
		}

		ContextMenuNode& FindOrCreateContainer(List<ContextMenuNode>& siblings, const std::string& name)
		{
			for (auto& child : siblings)
			{
				if (child.pItem == nullptr && child.displayName == name)
					return child;
			}

			ContextMenuNode& node = siblings.EmplaceBack();
			node.displayName = name;
			node.order = i32_max;
			node.pItem = nullptr;

			return node;
		}

		void ClearRecursive(ContextMenuNode& node)
		{
			Memory::Allocator::Delete(node.pItem);
			node.pItem = nullptr;

			for (auto& child : node.children)
				ClearRecursive(child);

			node.children.Clear();
		}

		void SortRecursive(List<ContextMenuNode>& siblings)
		{
			siblings.Sort([](const ContextMenuNode& a, const ContextMenuNode& b)
				{
					if (a.order != b.order)
						return a.order < b.order;

					return a.displayName < b.displayName;
				});

			for (auto& child : siblings)
				SortRecursive(child.children);
		}

		void RenderMenuNode(const ContextMenuNode& menu, TContext& context)
		{
			if (menu.pItem == nullptr)
			{
				if (ImGui::BeginMenu(menu.displayName.c_str(), !menu.children.IsEmpty()))
				{
					for (auto& child : menu.children)
						RenderMenuNode(child, context);

					ImGui::EndMenu();
				}

				return;
			}

			b8 enabled = menu.pItem->IsEnabled(context);

			if (ImGui::MenuItem(menu.displayName.c_str(), nullptr, false, enabled))
				menu.pItem->OnExecute(context);
		}

	private:
		std::string m_ownerId;
		List<ContextMenuNode> m_menus;
	};
}