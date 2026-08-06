#pragma once

#include <Editor/Attributes/ContextMenuItemAttribute.h>
#include <Editor/ContextMenu/ContextMenuItem.h>

#include <Engine/Core/Engine.h>
#include <Engine/Module/ModuleContext.h>

#include <Runtime/Containers/List.h>
#include <Runtime/Definitions/Allocator.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/RTTR/Reflection.h>

#include <imgui.h>

#include <string>

namespace Horizon
{
	template<typename TContext>
	struct ContextMenuNode
	{
		std::string displayName;
		i32 order = 0;

		ContextMenuItem<TContext>* item = nullptr;
		List<ContextMenuNode<TContext>> children;
	};

	template<typename TContext>
	class ContextMenuRegistry
	{
	public:
		ContextMenuRegistry() = default;

		~ContextMenuRegistry()
		{
			for (auto& node : m_nodes)
				ClearRecursive(node);

			m_nodes.Clear();
		}

		void Bootstrap(Engine* pEngine, const char* ownerId)
		{
			for (auto& node : m_nodes)
				ClearRecursive(node);

			m_nodes.Clear();

			auto* moduleCtx = pEngine->GetModuleContext();
			List<Reflect::Type*> types = moduleCtx->GetTypeByAttribute(Reflect::TypeOf<ContextMenuItemAttribute>());

			for (auto* type : types)
			{
				auto* attr = type->GetCustomAttribute<ContextMenuItemAttribute>();
				const std::string& path = attr->GetPath();

				const usize ownerSlash = path.find('/');

				if (ownerSlash == std::string::npos)
				{
					Terminal::Warn("ContextMenuRegistry", "{} has no owner segment", path);
					continue;
				}

				if (path.compare(0, ownerSlash, ownerId) != 0)
					continue;

				InsertItem(type, attr, path, ownerSlash + 1);
			}

			SortRecursive(m_nodes);
		}

		void RenderPopup(const char* strId, TContext& context)
		{
			if (ImGui::BeginPopupContextWindow(strId, ImGuiPopupFlags_MouseButtonRight))
			{
				for (auto& node : m_nodes)
					RenderNode(node, context);

				ImGui::EndPopup();
			}
		}

	private:
		void InsertItem(Reflect::Type* type, ContextMenuItemAttribute* attr, const std::string& path, usize start)
		{
			const i32 order = attr->GetOrder();

			List<ContextMenuNode<TContext>>* level = &m_nodes;

			while (true)
			{
				const usize slash = path.find('/', start);
				const b8 isLeaf = (slash == std::string::npos);
				std::string segment = path.substr(start, isLeaf ? std::string::npos : slash - start);

				if (isLeaf)
				{
					auto* menuObj = static_cast<ContextMenuItem<TContext>*>(type->CreateFromMemory());

					ContextMenuNode<TContext> leaf;
					leaf.displayName = std::move(segment);
					leaf.order = order;
					leaf.item = menuObj;

					level->PushBack(std::move(leaf));
					return;
				}

				ContextMenuNode<TContext>& container = FindOrCreateContainer(*level, segment);

				if (order < container.order)
					container.order = order;

				level = &container.children;
				start = slash + 1;
			}
		}

		ContextMenuNode<TContext>& FindOrCreateContainer(List<ContextMenuNode<TContext>>& siblings, const std::string& name)
		{
			for (auto& child : siblings)
			{
				if (child.displayName == name)
					return child;
			}

			ContextMenuNode<TContext> node;
			node.displayName = name;
			node.order = 0x7FFFFFFF;
			node.item = nullptr;

			return siblings.EmplaceBack(std::move(node));
		}

		void RenderNode(ContextMenuNode<TContext>& node, TContext& context)
		{
			if (node.item == nullptr)
			{
				if (ImGui::BeginMenu(node.displayName.c_str(), !node.children.IsEmpty()))
				{
					for (auto& child : node.children)
						RenderNode(child, context);

					ImGui::EndMenu();
				}

				return;
			}

			const b8 enabled = node.item->IsEnabled(context);

			if (ImGui::MenuItem(node.displayName.c_str(), nullptr, false, enabled))
				node.item->OnExecute(context);
		}

		void SortRecursive(List<ContextMenuNode<TContext>>& siblings)
		{
			siblings.Sort([](const ContextMenuNode<TContext>& a, const ContextMenuNode<TContext>& b)
				{
					if (a.order != b.order)
						return a.order < b.order;

					return a.displayName < b.displayName;
				});

			for (auto& child : siblings)
				SortRecursive(child.children);
		}

		void ClearRecursive(ContextMenuNode<TContext>& node)
		{
			Allocator::Delete(node.item);

			for (auto& child : node.children)
				ClearRecursive(child);
		}

	private:
		List<ContextMenuNode<TContext>> m_nodes;
	};
}