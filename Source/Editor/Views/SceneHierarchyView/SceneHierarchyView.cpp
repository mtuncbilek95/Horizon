#include "SceneHierarchyView.h"

#include <Editor/Components/Helpers/EntityPickerWidget.h>
#include <Editor/Models/SelectionModel.h>
#include <Editor/Renderer/EditorContext.h>
#include <Editor/Domain/DomainFile.h>
#include <Editor/Font/FontAwesomeIconCache.h>

#include <Engine/Asset/Scene/SceneSerializer.h>
#include <Engine/Job/JobSystem.h>
#include <Engine/World/WorldService.h>
#include <Engine/World/Components/Tag/NameComponent.h>
#include <Engine/World/Components/EditorOnlyComponent.h>
#include <Engine/World/Components/FolderComponent.h>

#include <Runtime/PAL/File/File.h>
#include <Runtime/Containers/StringOps.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/Serialization/JsonArchive.h>

namespace Horizon::Editor
{
	namespace
	{
		static constexpr std::string_view sContextName = "SceneHierarchyContext";
		static constexpr std::string_view sPopupName = "Rename - Scene Hierarchy";
		static constexpr std::string_view sIconPopupName = "Icon - Scene Hierarchy";
		static constexpr std::string_view sDeletePopupName = "Delete Folder - Scene Hierarchy";
		static constexpr std::string_view sNewFolderName = "New Folder";

		ImGuiID FolderId(const std::string& path)
		{
			const std::string strId = "##folder/" + path;
			return ImGui::GetID(strId.c_str());
		}

		std::string ParentOf(const std::string& path)
		{
			const usize slash = path.rfind('/');

			if (slash == std::string::npos)
				return std::string();

			return path.substr(0, slash);
		}

		std::string LeafOf(const std::string& path)
		{
			const usize slash = path.rfind('/');

			if (slash == std::string::npos)
				return path;

			return path.substr(slash + 1);
		}

		std::string JoinPath(const std::string& parent, const std::string& leaf)
		{
			if (parent.empty())
				return leaf;

			return parent + "/" + leaf;
		}

		std::string Rebase(const std::string& path, const std::string& from, const std::string& to)
		{
			if (path == from)
				return to;

			if (!path.starts_with(from + "/"))
				return path;

			if (to.empty())
				return path.substr(from.size() + 1);

			return to + path.substr(from.size());
		}

		b8 IsUnder(const std::string& path, const std::string& folder)
		{
			return path == folder || path.starts_with(folder + "/");
		}

		void IndentRow(u32 depth)
		{
			if (depth == 0)
				return;

			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + static_cast<f32>(depth) * ImGui::GetStyle().IndentSpacing);
		}
	}

	void SceneHierarchyView::OnInvoke()
	{
		m_context.BootstrapContext(GetContext()->pEngine, "SceneHierarchyView");
	}

	void SceneHierarchyView::OnRender(const Engine::EngineFrame& context)
	{
		if (!m_connectedFile)
		{
			ImGui::TextDisabled("No active world");
			return;
		}
		else
		{
			auto* pWorldService = GetContext()->pEngine->RequestService<Engine::WorldService>();

			if (m_currentScene != pWorldService->GetCurrentWorld())
			{
				ResetSceneState();
				m_currentScene = pWorldService->GetCurrentWorld();
			}
		}

		CollectEntities();
		AttachMissingFolders();
		BuildTree();

		std::erase_if(m_selectedFolders, [this](const std::string& folder)
			{
				return !m_tree.contains(folder);
			});

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 8.0f));
		const b8 frameVisible = ImGui::BeginChild("##HierarchyFrame", ImVec2(0.0f, 0.0f), ImGuiChildFlags_AlwaysUseWindowPadding);
		ImGui::PopStyleVar();

		if (frameVisible)
		{
			if (!m_openFolder.empty())
			{
				ImGui::GetStateStorage()->SetInt(FolderId(m_openFolder), 1);
				m_openFolder.clear();
			}

			m_rows.Clear();
			m_entities.Clear();
			BuildRows(std::string(), 0);

			ImGuiMultiSelectFlags msFlags = ImGuiMultiSelectFlags_ClearOnEscape | ImGuiMultiSelectFlags_SelectOnClickRelease;

			if (!ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel))
				msFlags |= ImGuiMultiSelectFlags_BoxSelect1d | ImGuiMultiSelectFlags_ClearOnClickVoid;

			ImGuiMultiSelectIO* pMultiIO = ImGui::BeginMultiSelect(msFlags, m_selection.Size, static_cast<i32>(m_entities.GetCount()));

			m_selection.UserData = this;
			m_selection.AdapterIndexToStorageId = [](ImGuiSelectionBasicStorage* pSelf, int index)
				{
					SceneHierarchyView* pView = static_cast<SceneHierarchyView*>(pSelf->UserData);
					return static_cast<ImGuiID>(pView->m_entities[static_cast<usize>(index)].Index());
				};

			ApplySelectionRequests(pMultiIO);

			usize selectionIndex = 0;

			for (usize i = 0; i < m_rows.GetCount(); i++)
			{
				if (m_rows[i].entity.IsValid())
					RenderEntityRow(m_rows[i], selectionIndex++);
				else
					RenderFolderRow(m_rows[i]);
			}

			pMultiIO = ImGui::EndMultiSelect();
			ApplySelectionRequests(pMultiIO);

			RenderVoidTarget();

			SceneHierarchyContext sceneContext = {};
			sceneContext.pEngine = GetContext()->pEngine;
			sceneContext.pCurrentScene = m_currentScene;

			void* pIt = nullptr;
			ImGuiID selectedId = 0;

			while (m_selection.GetNextSelectedItem(&pIt, &selectedId))
			{
				const Engine::EntityHandle entt = m_currentScene->GetEntities().GetHandleAt(selectedId);

				if (entt.IsValid())
					sceneContext.selectedEntities.PushBack(entt);
			}

			for (const std::string& folder : m_selectedFolders)
				sceneContext.selectedFolders.PushBack(folder);

			if (m_pendingMove.isValid)
				ApplyMove(sceneContext.selectedEntities);

			if (sceneContext.selectedEntities.GetCount() == 1)
				GetContext()->pSelection->Set<Engine::EntityTag>(sceneContext.selectedEntities[0]);
			else
				GetContext()->pSelection->Clear();

			m_context.RenderGUI(sContextName.data(), sceneContext);

			if (sceneContext.renameEntity.IsValid())
				BeginRename(sceneContext.renameEntity);

			if (sceneContext.newIcon.IsValid())
				BeginIconPick(sceneContext.newIcon);

			if (sceneContext.createFolder)
				CreateFolder(sceneContext.selectedEntities);

			if (sceneContext.deleteFolder)
				BeginFolderDelete();

			RenderRenameModal();
			RenderIconModal();
			RenderDeleteFolderModal();
		}

		ImGui::EndChild();
	}

	b8 SceneHierarchyView::OnCommand(ViewCommand command)
	{
		if (command != ViewCommand::Save)
			return false;

		SaveSceneToSource();
		return true;
	}

	void SceneHierarchyView::OnLibraryRegistered(const Engine::ReflectionLibrary& library)
	{
		m_context.OnLibraryRegistered(library);
	}

	void SceneHierarchyView::OnLibraryUnregistered(const Engine::ReflectionLibrary& library)
	{
		m_context.OnLibraryUnregistered(library);
		ResetSelectionState();
	}

	void SceneHierarchyView::ResetSceneState()
	{
		ResetSelectionState();
		m_sessionFolders.clear();
		m_selectedFolders.clear();
		m_deleteFolders.clear();
		m_openFolder.clear();
		m_renamePath.clear();
		m_renameFolder.clear();
	}

	void SceneHierarchyView::ResetSelectionState()
	{
		if (GetContext()->pSelection->Is<Engine::EntityTag>())
			GetContext()->pSelection->Clear();

		m_selection.Clear();
		m_pendingMove = PendingMove();
		m_renameHandl = Engine::EntityHandle();
		m_iconHandl = Engine::EntityHandle();
		m_iconSelection.clear();
		m_iconMatches.Clear();
	}

	void SceneHierarchyView::CollectEntities()
	{
		m_sceneEntities.Clear();
		m_currentScene->ForEach<Engine::NameComponent>([&](Engine::EntityHandle entt, const Engine::NameComponent&)
			{
				if (!m_currentScene->HasComponent<Engine::EditorOnlyComponent>(entt))
					m_sceneEntities.PushBack(entt);
			});
	}

	void SceneHierarchyView::AttachMissingFolders()
	{
		List<Engine::EntityHandle> missing;
		u32 nextOrder = 0;

		for (usize i = 0; i < m_sceneEntities.GetCount(); i++)
		{
			const Engine::FolderComponent* pFolder = m_currentScene->FindComponent<Engine::FolderComponent>(m_sceneEntities[i]);

			if (!pFolder)
			{
				missing.PushBack(m_sceneEntities[i]);
				continue;
			}

			if (pFolder->m_path.empty())
				nextOrder = std::max(nextOrder, pFolder->m_order + 1);
		}

		for (usize i = 0; i < missing.GetCount(); i++)
		{
			auto* pFolder = m_currentScene->AddComponent(missing[i], Engine::FolderComponent());
			pFolder->m_path.clear();
			pFolder->m_order = nextOrder++;
		}
	}

	void SceneHierarchyView::BuildTree()
	{
		m_tree.clear();
		m_tree[std::string()];

		for (const std::string& folder : m_sessionFolders)
			EnsureFolder(folder);

		for (usize i = 0; i < m_sceneEntities.GetCount(); i++)
		{
			const Engine::EntityHandle entt = m_sceneEntities[i];
			const Engine::FolderComponent* pFolder = m_currentScene->FindComponent<Engine::FolderComponent>(entt);

			EnsureFolder(pFolder->m_path);
			m_tree[pFolder->m_path].entities.PushBack(entt);
		}

		for (auto& [path, node] : m_tree)
		{
			node.entities.Sort([this](Engine::EntityHandle left, Engine::EntityHandle right)
				{
					return IsBefore(left, right);
				});
		}
	}

	void SceneHierarchyView::EnsureFolder(const std::string& path)
	{
		if (m_tree.contains(path))
			return;

		const std::string parent = ParentOf(path);

		EnsureFolder(parent);
		m_tree[parent].folders.insert(path);
		m_tree[path];
	}

	void SceneHierarchyView::BuildRows(const std::string& folder, u32 depth)
	{
		auto it = m_tree.find(folder);

		if (it == m_tree.end())
		{
			Terminal::Error(StringOps::GetName(this), "{} folder has no tree node", folder);
			return;
		}

		const FolderNode& node = it->second;

		for (const std::string& child : node.folders)
		{
			m_rows.PushBack(HierarchyRow{ child, Engine::EntityHandle(), depth });

			if (ImGui::GetStateStorage()->GetInt(FolderId(child), 0) != 0)
				BuildRows(child, depth + 1);
		}

		for (usize i = 0; i < node.entities.GetCount(); i++)
		{
			m_rows.PushBack(HierarchyRow{ folder, node.entities[i], depth });
			m_entities.PushBack(node.entities[i]);
		}
	}

	b8 SceneHierarchyView::IsBefore(Engine::EntityHandle left, Engine::EntityHandle right) const
	{
		const Engine::FolderComponent* pLeft = m_currentScene->FindComponent<Engine::FolderComponent>(left);
		const Engine::FolderComponent* pRight = m_currentScene->FindComponent<Engine::FolderComponent>(right);

		if (pLeft->m_path != pRight->m_path)
			return pLeft->m_path < pRight->m_path;

		if (pLeft->m_order != pRight->m_order)
			return pLeft->m_order < pRight->m_order;

		return left.Index() < right.Index();
	}

	void SceneHierarchyView::RenderFolderRow(const HierarchyRow& row)
	{
		const std::string strId = "##folder/" + row.folder;
		const b8 isOpen = ImGui::GetStateStorage()->GetInt(FolderId(row.folder), 0) != 0;
		const b8 isSelected = m_selectedFolders.contains(row.folder);
		const std::string label = std::format("{} {}", isOpen ? ICON_FA_FOLDER_OPEN : ICON_FA_FOLDER, LeafOf(row.folder));

		IndentRow(row.depth);

		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;

		if (isSelected)
			flags |= ImGuiTreeNodeFlags_Selected;

		ImGui::TreeNodeEx(strId.c_str(), flags, "%s", label.c_str());

		if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && !ImGui::IsItemToggledOpen())
			SelectFolder(row.folder, ImGui::GetIO().KeyCtrl);

		if (ImGui::IsItemClicked(ImGuiMouseButton_Right) && !isSelected)
			SelectFolder(row.folder, false);

		if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && !ImGui::IsItemToggledOpen())
			BeginFolderRename(row.folder);

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* pPayload = ImGui::AcceptDragDropPayload(EntityPickerWidget::PayloadName))
				QueueMove(pPayload, row.folder, Engine::EntityHandle(), false);

			ImGui::EndDragDropTarget();
		}
	}

	void SceneHierarchyView::RenderEntityRow(const HierarchyRow& row, usize selectionIndex)
	{
		const Engine::EntityHandle entt = row.entity;
		const u32 id = entt.Index();
		Engine::NameComponent* pNameComp = m_currentScene->FindComponent<Engine::NameComponent>(entt);

		std::string name;
		name += pNameComp->m_typeIcon.empty() ? ICON_FA_CUBE " " : pNameComp->m_typeIcon + " ";
		name += pNameComp->m_name;

		IndentRow(row.depth);

		ImGui::PushID(i32(id));
		ImGui::SetNextItemSelectionUserData(i64(selectionIndex));
		ImGui::Selectable(name.data(), m_selection.Contains(id));

		if (ImGui::BeginDragDropSource())
		{
			ImGui::SetDragDropPayload(EntityPickerWidget::PayloadName, &entt, sizeof(Engine::EntityHandle));
			ImGui::TextUnformatted(name.c_str());
			ImGui::EndDragDropSource();
		}

		if (ImGui::BeginDragDropTarget())
		{
			const ImVec2 rectMin = ImGui::GetItemRectMin();
			const ImVec2 rectMax = ImGui::GetItemRectMax();
			const b8 after = ImGui::GetMousePos().y > (rectMin.y + rectMax.y) * 0.5f;

			const ImGuiDragDropFlags dropFlags = ImGuiDragDropFlags_AcceptBeforeDelivery | ImGuiDragDropFlags_AcceptNoDrawDefaultRect;

			if (const ImGuiPayload* pPayload = ImGui::AcceptDragDropPayload(EntityPickerWidget::PayloadName, dropFlags))
			{
				const f32 lineY = after ? rectMax.y : rectMin.y;
				ImGui::GetWindowDrawList()->AddLine(ImVec2(rectMin.x, lineY), ImVec2(rectMax.x, lineY), ImGui::GetColorU32(ImGuiCol_DragDropTarget), 2.0f);

				if (pPayload->IsDelivery())
					QueueMove(pPayload, row.folder, entt, after);
			}

			ImGui::EndDragDropTarget();
		}

		ImGui::PopID();
	}

	void SceneHierarchyView::RenderVoidTarget()
	{
		const ImVec2 avail = ImGui::GetContentRegionAvail();
		ImGui::Dummy(ImVec2(avail.x, std::max(avail.y, ImGui::GetFrameHeight())));

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* pPayload = ImGui::AcceptDragDropPayload(EntityPickerWidget::PayloadName))
				QueueMove(pPayload, std::string(), Engine::EntityHandle(), false);

			ImGui::EndDragDropTarget();
		}
	}

	void SceneHierarchyView::ApplySelectionRequests(ImGuiMultiSelectIO* pMultiIO)
	{
		for (const ImGuiSelectionRequest& request : pMultiIO->Requests)
		{
			if (request.Type == ImGuiSelectionRequestType_SetAll && !request.Selected)
				m_selectedFolders.clear();
		}

		m_selection.ApplyRequests(pMultiIO);
	}

	void SceneHierarchyView::SelectFolder(const std::string& path, b8 additive)
	{
		if (!additive)
		{
			m_selection.Clear();
			m_selectedFolders.clear();
		}

		if (additive && m_selectedFolders.contains(path))
			m_selectedFolders.erase(path);
		else
			m_selectedFolders.insert(path);
	}

	void SceneHierarchyView::QueueMove(const ImGuiPayload* pPayload, const std::string& folder, Engine::EntityHandle anchor, b8 after)
	{
		m_pendingMove.folder = folder;
		m_pendingMove.dragged = *static_cast<const Engine::EntityHandle*>(pPayload->Data);
		m_pendingMove.anchor = anchor;
		m_pendingMove.after = after;
		m_pendingMove.isValid = true;
	}

	void SceneHierarchyView::ApplyMove(const List<Engine::EntityHandle>& selected)
	{
		const PendingMove move = m_pendingMove;
		m_pendingMove = PendingMove();

		if (!m_currentScene->IsAlive(move.dragged))
			return;

		List<Engine::EntityHandle> moving;

		if (selected.Contains(move.dragged))
			moving = selected;
		else
			moving.PushBack(move.dragged);

		if (move.anchor.IsValid() && moving.Contains(move.anchor))
			return;

		moving.Sort([this](Engine::EntityHandle left, Engine::EntityHandle right)
			{
				return IsBefore(left, right);
			});

		List<Engine::EntityHandle> siblings;

		m_currentScene->ForEach<Engine::FolderComponent>([&](Engine::EntityHandle entt, const Engine::FolderComponent& folder)
			{
				if (folder.m_path == move.folder && !moving.Contains(entt))
					siblings.PushBack(entt);
			});

		siblings.Sort([this](Engine::EntityHandle left, Engine::EntityHandle right)
			{
				return IsBefore(left, right);
			});

		usize insertAt = siblings.GetCount();

		if (move.anchor.IsValid())
		{
			const i64 anchorIndex = siblings.FindIndex(move.anchor);

			if (anchorIndex != -1)
				insertAt = static_cast<usize>(anchorIndex) + (move.after ? 1 : 0);
		}

		for (usize i = 0; i < moving.GetCount(); i++)
			siblings.PushAt(insertAt + i, moving[i]);

		for (usize i = 0; i < siblings.GetCount(); i++)
		{
			auto* pFolder = m_currentScene->FindComponent<Engine::FolderComponent>(siblings[i]);
			pFolder->m_path = move.folder;
			pFolder->m_order = static_cast<u32>(i);
		}
	}

	void SceneHierarchyView::CreateFolder(const List<Engine::EntityHandle>& selected)
	{
		std::string parent;

		if (!selected.IsEmpty())
		{
			const Engine::FolderComponent* pFolder = m_currentScene->FindComponent<Engine::FolderComponent>(selected[0]);

			if (pFolder)
				parent = pFolder->m_path;
		}

		std::string path = JoinPath(parent, std::string(sNewFolderName));

		for (u32 suffix = 1; m_tree.contains(path); suffix++)
			path = JoinPath(parent, std::format("{} ({})", sNewFolderName, suffix));

		m_sessionFolders.insert(path);
		EnsureFolder(path);

		if (!selected.IsEmpty())
		{
			m_pendingMove.folder = path;
			m_pendingMove.dragged = selected[0];
			m_pendingMove.anchor = Engine::EntityHandle();
			m_pendingMove.after = false;
			m_pendingMove.isValid = true;

			ApplyMove(selected);
		}

		m_openFolder = path;
		BeginFolderRename(path);
	}

	void SceneHierarchyView::RenameFolder(const std::string& path, const std::string& leaf)
	{
		if (leaf.empty() || leaf.find('/') != std::string::npos)
		{
			Terminal::Warn(StringOps::GetName(this), "{} is not a valid folder name", leaf);
			return;
		}

		const std::string renamed = JoinPath(ParentOf(path), leaf);

		if (renamed == path)
			return;

		if (m_tree.contains(renamed))
		{
			Terminal::Warn(StringOps::GetName(this), "{} folder already exists", renamed);
			return;
		}

		m_currentScene->ForEach<Engine::FolderComponent>([&](Engine::EntityHandle, Engine::FolderComponent& folder)
			{
				folder.m_path = Rebase(folder.m_path, path, renamed);
			});

		std::set<std::string> sessionFolders;

		for (const std::string& folder : m_sessionFolders)
			sessionFolders.insert(Rebase(folder, path, renamed));

		m_sessionFolders.swap(sessionFolders);

		std::set<std::string> selectedFolders;

		for (const std::string& folder : m_selectedFolders)
			selectedFolders.insert(Rebase(folder, path, renamed));

		m_selectedFolders.swap(selectedFolders);
		m_openFolder = renamed;
	}

	b8 SceneHierarchyView::HasEntitiesUnder(const std::string& path) const
	{
		for (usize i = 0; i < m_sceneEntities.GetCount(); i++)
		{
			const Engine::FolderComponent* pFolder = m_currentScene->FindComponent<Engine::FolderComponent>(m_sceneEntities[i]);

			if (pFolder && IsUnder(pFolder->m_path, path))
				return true;
		}

		return false;
	}

	void SceneHierarchyView::BeginFolderDelete()
	{
		if (m_selectedFolders.empty() || !m_deleteFolders.empty())
			return;

		m_deleteFolders = m_selectedFolders;

		for (const std::string& folder : m_deleteFolders)
		{
			if (HasEntitiesUnder(folder))
			{
				ImGui::OpenPopup(sDeletePopupName.data());
				return;
			}
		}

		DeleteFolders(false);
	}

	void SceneHierarchyView::DeleteFolders(b8 withInside)
	{
		for (auto it = m_deleteFolders.rbegin(); it != m_deleteFolders.rend(); ++it)
		{
			const std::string folder = *it;
			const std::string parent = ParentOf(folder);

			if (withInside)
			{
				List<Engine::EntityHandle> removed;

				m_currentScene->ForEach<Engine::FolderComponent>([&](Engine::EntityHandle entt, const Engine::FolderComponent& folderComp)
					{
						if (IsUnder(folderComp.m_path, folder))
							removed.PushBack(entt);
					});

				for (usize i = 0; i < removed.GetCount(); i++)
					m_currentScene->RemoveEntity(removed[i]);
			}
			else
			{
				List<Engine::EntityHandle> lifted;
				u32 nextOrder = 0;

				m_currentScene->ForEach<Engine::FolderComponent>([&](Engine::EntityHandle entt, Engine::FolderComponent& folderComp)
					{
						if (folderComp.m_path == folder)
							lifted.PushBack(entt);
						else if (folderComp.m_path == parent)
							nextOrder = std::max(nextOrder, folderComp.m_order + 1);
						else
							folderComp.m_path = Rebase(folderComp.m_path, folder, parent);
					});

				lifted.Sort([this](Engine::EntityHandle left, Engine::EntityHandle right)
					{
						return IsBefore(left, right);
					});

				for (usize i = 0; i < lifted.GetCount(); i++)
				{
					auto* pFolder = m_currentScene->FindComponent<Engine::FolderComponent>(lifted[i]);
					pFolder->m_path = parent;
					pFolder->m_order = nextOrder++;
				}
			}

			std::set<std::string> sessionFolders;

			for (const std::string& sessionFolder : m_sessionFolders)
			{
				if (sessionFolder == folder)
					continue;

				if (withInside && IsUnder(sessionFolder, folder))
					continue;

				sessionFolders.insert(Rebase(sessionFolder, folder, parent));
			}

			m_sessionFolders.swap(sessionFolders);
		}

		if (withInside)
		{
			m_selection.Clear();
			GetContext()->pSelection->Clear();
		}

		m_selectedFolders.clear();
		m_deleteFolders.clear();
	}

	void SceneHierarchyView::RenderDeleteFolderModal()
	{
		if (!ImGui::BeginPopupModal(sDeletePopupName.data(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
			return;

		ImGui::TextUnformatted(ICON_FA_TRIANGLE_EXCLAMATION " Selected folder(s) are not empty.");
		ImGui::TextUnformatted("Just Folder moves the contents to the parent folder.");
		ImGui::TextUnformatted("With Inside deletes every entity and sub-folder inside.");
		ImGui::Spacing();

		if (ImGui::Button("Just Folder"))
		{
			DeleteFolders(false);
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("With Inside"))
		{
			DeleteFolders(true);
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(ImGuiKey_Escape))
		{
			m_deleteFolders.clear();
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	void SceneHierarchyView::BeginRename(Engine::EntityHandle handl)
	{
		if (!m_renamePath.empty())
			return;

		if (!m_currentScene)
			return;

		auto* pNameComp = m_currentScene->FindComponent<Engine::NameComponent>(handl);
		m_renameHandl = handl;
		m_renamePath = pNameComp->m_name;

		std::snprintf(m_renameBuffer, sizeof(m_renameBuffer), "%s", pNameComp->m_name.data());
		ImGui::OpenPopup(sPopupName.data());
	}

	void SceneHierarchyView::BeginFolderRename(const std::string& path)
	{
		if (!m_renamePath.empty())
			return;

		m_renameFolder = path;
		m_renamePath = path;

		std::snprintf(m_renameBuffer, sizeof(m_renameBuffer), "%s", LeafOf(path).c_str());
		ImGui::OpenPopup(sPopupName.data());
	}

	void SceneHierarchyView::RenderRenameModal()
	{
		if (!ImGui::BeginPopupModal(sPopupName.data(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
			return;

		b8 accepted = ImGui::InputText("##name", m_renameBuffer, sizeof(m_renameBuffer), ImGuiInputTextFlags_EnterReturnsTrue);

		if (ImGui::Button("OK") || accepted)
		{
			if (!m_renameFolder.empty())
			{
				RenameFolder(m_renameFolder, m_renameBuffer);
			}
			else
			{
				auto* pNameComp = m_currentScene->FindComponent<Engine::NameComponent>(m_renameHandl);
				pNameComp->m_name = m_renameBuffer;
			}

			m_renamePath.clear();
			m_renameFolder.clear();
			m_renameHandl = Engine::EntityHandle();
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(ImGuiKey_Escape))
		{
			m_renamePath.clear();
			m_renameFolder.clear();
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	void SceneHierarchyView::BeginIconPick(Engine::EntityHandle handl)
	{
		if (m_iconHandl.IsValid())
			return;

		if (!m_currentScene)
			return;

		auto* pNameComp = m_currentScene->FindComponent<Engine::NameComponent>(handl);

		if (!pNameComp)
		{
			Terminal::Error(StringOps::GetName(this), "{} entity has no NameComponent", handl.Index());
			return;
		}

		m_iconHandl = handl;
		m_iconSelection = pNameComp->m_typeIcon;
		m_iconFilter[0] = '\0';

		RebuildIconMatches();
		ImGui::OpenPopup(sIconPopupName.data());
	}

	void SceneHierarchyView::RebuildIconMatches()
	{
		std::string filter = m_iconFilter;

		for (c8& ch : filter)
			ch = static_cast<c8>(std::tolower(static_cast<u8>(ch)));

		m_iconMatches.Clear();

		for (const auto& [name, glyph] : FontAwesomeIconCache)
		{
			if (filter.empty() || name.find(filter) != std::string_view::npos)
				m_iconMatches.PushBack({ name, glyph });
		}

		m_iconMatches.Sort([](const auto& left, const auto& right)
			{
				return left.first < right.first;
			});
	}

	void SceneHierarchyView::RenderIconModal()
	{
		if (!ImGui::BeginPopupModal(sIconPopupName.data(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
			return;

		const ImGuiStyle& style = ImGui::GetStyle();
		const f32 cell = ImGui::GetFrameHeight() * 1.6f;
		b8 accepted = false;

		ImGui::SetNextItemWidth(cell * 14.0f);

		if (ImGui::InputTextWithHint("##filter", "Search", m_iconFilter, sizeof(m_iconFilter)))
			RebuildIconMatches();

		if (ImGui::BeginChild("##icons", ImVec2(cell * 14.0f, cell * 9.0f), ImGuiChildFlags_Borders))
		{
			const i32 count = static_cast<i32>(m_iconMatches.GetCount());
			const i32 columns = std::max(1, static_cast<i32>(ImGui::GetContentRegionAvail().x / (cell + style.ItemSpacing.x)));
			const i32 rows = (count + columns - 1) / columns;

			ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.5f, 0.5f));

			ImGuiListClipper clipper;
			clipper.Begin(rows, cell + style.ItemSpacing.y);

			while (clipper.Step())
			{
				for (i32 row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row)
				{
					for (i32 col = 0; col < columns; ++col)
					{
						const i32 index = row * columns + col;

						if (index >= count)
							break;

						const auto& [name, glyph] = m_iconMatches[index];

						if (col > 0)
							ImGui::SameLine();

						ImGui::PushID(index);

						if (ImGui::Selectable(glyph.data(), m_iconSelection == glyph, ImGuiSelectableFlags_None, ImVec2(cell, cell)))
							m_iconSelection = glyph;

						if (ImGui::IsItemHovered())
						{
							ImGui::SetTooltip("%s", name.data());

							if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
								accepted = true;
						}

						ImGui::PopID();
					}
				}
			}

			ImGui::PopStyleVar();
		}

		ImGui::EndChild();

		if (ImGui::Button("OK") || accepted)
		{
			auto* pNameComp = m_currentScene->FindComponent<Engine::NameComponent>(m_iconHandl);
			pNameComp->m_typeIcon = m_iconSelection;

			CloseIconModal();
		}

		ImGui::SameLine();

		if (ImGui::Button("Default"))
			m_iconSelection.clear();

		ImGui::SameLine();

		if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(ImGuiKey_Escape))
			CloseIconModal();

		ImGui::EndPopup();
	}

	void SceneHierarchyView::CloseIconModal()
	{
		m_iconHandl = Engine::EntityHandle();
		m_iconSelection.clear();
		m_iconMatches.Clear();
		ImGui::CloseCurrentPopup();
	}

	void SceneHierarchyView::SaveSceneToSource()
	{
		if (!m_currentScene)
			return;

		auto* pJobSys = GetContext()->pEngine->GetJobSystem();
		Engine::ReflectionSystem* pReflection = GetContext()->pEngine->GetReflectionSystem();

		JsonArchiveWriter writer;
		Engine::SceneSerializer::Serialize(*m_currentScene, pReflection, writer);

		std::string text = writer.ToString();
		std::string path = GetConnectedFile()->GetSourcePath();

		pJobSys->SubmitJob(Engine::JobLane::Background, Engine::Job([text = std::move(text), path = std::move(path)]()
			{
				PAL::FileAccessRequest handle = PAL::File::RequestAccess(path,
					PAL::FileOperationAccessPolicy::Write, PAL::FileOperationSharePolicy::SharedWrite);

				PAL::File::WriteString(handle, text);
				PAL::File::Truncate(handle, text.size());

				PAL::File::ReleaseAccess(handle);
			}));
	}
}