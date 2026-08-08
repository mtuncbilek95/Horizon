#pragma once

#include <Editor/Attributes/ContextMenuItemAttribute.h>
#include <Editor/ContextMenu/ContextMenuItem.h>
#include <Engine/Core/Application.h>
#include <Engine/Module/ModuleContext.h>

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

			ContextMenuItem<TContext>* item = nullptr;
			List<ContextMenuNode> children;
		};
	public:
		~ContextMenuRegistry()
		{
			Clear();
		}

		void BootstrapContext(Engine::Application* pEngine, const std::string& ownerId)
		{
			Clear();

			auto* pModule = pEngine->GetModuleContext();

			List<Reflect::Type*> types = pModule->GetTypeByAttribute(Reflect::TypeOf<ContextMenuItemAttribute>());

			for (Reflect::Type* pType : types)
			{
				auto* pAttr = pType->GetCustomAttribute<ContextMenuItemAttribute>();

				if (pAttr == nullptr)
				{
					Terminal::Error("ContextMenuRegistry", "{} type has no ContextMenuItemAttribute", pType->GetName());
					continue;
				}

				if (pAttr->GetOwner() != ownerId)
					continue;

				InsertMenu(pType, pAttr);
			}

			SortRecursive(m_menus);
		}

		void RenderGUI(std::string_view name, const TContext& context)
		{
			if (ImGui::BeginPopupContextWindow(name.data(), ImGuiPopupFlags_MouseButtonRight))
			{
				for (auto& menu : m_menus)
					RenderMenuNode(menu, context);

				ImGui::EndPopup();
			}
		}

	private:
		void Clear()
		{
			for (auto& node : m_menus)
				ClearRecursive(node);

			m_menus.Clear();
		}

		void InsertMenu(Reflect::Type* pType, ContextMenuItemAttribute* pAttr)
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
					leaf.item = static_cast<ContextMenuItem<TContext>*>(pType->CreateFromMemory());
					break;
				}

				ContextMenuNode& container = FindOrCreateContainer(*pLevel, segment);

				if (order < container.order)
					container.order = order;

				pLevel = &container.children;
				start = slash + 1;
			}
		}

		ContextMenuNode& FindOrCreateContainer(List<ContextMenuNode>& siblings, const std::string& name)
		{
			for (auto& child : siblings)
			{
				if (child.item == nullptr && child.displayName == name)
					return child;
			}

			ContextMenuNode& node = siblings.EmplaceBack();
			node.displayName = name;
			node.order = i32_max;
			node.item = nullptr;

			return node;
		}

		void ClearRecursive(ContextMenuNode& node)
		{
			Memory::Allocator::Delete(node.item);
			node.item = nullptr;

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

		void RenderMenuNode(const ContextMenuNode& menu, const TContext& context)
		{
			if (menu.item == nullptr)
			{
				if (ImGui::BeginMenu(menu.displayName.c_str(), !menu.children.IsEmpty()))
				{
					for (auto& child : menu.children)
						RenderMenuNode(child, context);

					ImGui::EndMenu();
				}

				return;
			}

			b8 enabled = menu.item->IsEnabled(context);

			if (ImGui::MenuItem(menu.displayName.c_str(), nullptr, false, enabled))
				menu.item->OnExecute(context);
		}

	private:
		List<ContextMenuNode> m_menus;
	};
}