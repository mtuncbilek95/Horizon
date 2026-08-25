#include "SceneHierarchyView.h"

#include <Editor/Models/SelectionModel.h>
#include <Editor/Renderer/EditorContext.h>

#include <Engine/Core/Engine.h>
#include <Engine/World/World.h>
#include <Engine/World/WorldService.h>
#include <Engine/World/Components/NameComponent.h>

#include <Runtime/Containers/StringOps.h>
#include <Runtime/Log/Terminal.h>

namespace Horizon::Editor
{
	namespace
	{
		ImGuiID MakeRowId(Engine::EntityHandle entity)
		{
			return static_cast<ImGuiID>(std::hash<u64>{}(entity.index));
		}
	}

	void SceneHierarchyView::OnInvoke()
	{
		m_contextMenu.BootstrapContext(GetContext()->pEngine, "SceneHierarchyView");

		auto* pWorldService = GetContext()->pEngine->RequestService<Engine::WorldService>();

		if (pWorldService == nullptr)
		{
			Terminal::Error(StringOps::GetName(this), "WorldService is unavailable, hierarchy stays empty");
			return;
		}

		SetCurrentWorld(pWorldService->GetActiveWorld());
	}

	void SceneHierarchyView::OnRender()
	{
		if (m_currentWorld == nullptr)
		{
			ImGui::TextDisabled("No active world");
			return;
		}

		RebuildRows();
		RenderRows();
		SyncSelectionModel();

		SceneHierarchyContext context = {};
		context.pEngine = GetContext()->pEngine;

		CollectSelected(context);
		m_contextMenu.RenderGUI("SceneHierarchyContextMenu", context);
	}

	void SceneHierarchyView::SetCurrentWorld(Engine::World* pCurrentWorld)
	{
		// TODO: This function may need Load/Unload. Idk I'm just guessing. Didn't decide yet.
		m_currentWorld = pCurrentWorld;
		m_multiSelect.Clear();
		m_rows.Clear();
	}

	void SceneHierarchyView::RebuildRows()
	{
		m_rows.Clear();

		Engine::EntityStorage& entities = m_currentWorld->GetEntities();
		const u32 highWaterMark = entities.GetHighWaterMark();

		for (u32 i = 0; i < highWaterMark; i++)
		{
			Engine::EntityHandle entity = entities.GetHandleAt(i);

			if (!entities.IsAlive(entity))
				continue;

			HierarchyRow& row = m_rows.EmplaceBack();
			row.entity = entity;
			row.id = MakeRowId(entity);
		}
	}

	void SceneHierarchyView::RenderRows()
	{
		ImGuiMultiSelectFlags msFlags = ImGuiMultiSelectFlags_ClearOnEscape;

		if (!ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel))
		{
			msFlags |= ImGuiMultiSelectFlags_BoxSelect1d | ImGuiMultiSelectFlags_ClearOnClickVoid;
		}

		ImGuiMultiSelectIO* pMultiIO = ImGui::BeginMultiSelect(msFlags, m_multiSelect.Size, static_cast<i32>(m_rows.GetCount()));

		m_multiSelect.UserData = this;
		m_multiSelect.AdapterIndexToStorageId = [](ImGuiSelectionBasicStorage* pSelf, int index)
			{
				SceneHierarchyView* pView = static_cast<SceneHierarchyView*>(pSelf->UserData);
				return pView->m_rows[static_cast<usize>(index)].id;
			};

		m_multiSelect.ApplyRequests(pMultiIO);

		for (usize i = 0; i < m_rows.GetCount(); i++)
		{
			const HierarchyRow& row = m_rows[i];

			auto* pName = m_currentWorld->FindComponent<Engine::NameComponent>(row.entity);

			c8 fallback[32] = {};
			const c8* pLabel = nullptr;

			if (pName != nullptr && !pName->m_name.empty())
				pLabel = pName->m_name.c_str();
			else
			{
				std::snprintf(fallback, sizeof(fallback), ICON_FA_CUBE " Entity %u", row.entity.Index());
				pLabel = fallback;
			}

			ImGui::SetNextItemSelectionUserData(static_cast<ImGuiSelectionUserData>(i));
			ImGui::PushID(static_cast<i32>(row.id));
			ImGui::Selectable(pLabel, m_multiSelect.Contains(row.id));
			ImGui::PopID();
		}

		pMultiIO = ImGui::EndMultiSelect();
		m_multiSelect.ApplyRequests(pMultiIO);
	}

	void SceneHierarchyView::SyncSelectionModel()
	{
		SelectionModel* pSelection = GetContext()->pSelection;

		if (pSelection == nullptr)
		{
			Terminal::Error(StringOps::GetName(this), "Editor context carries no selection model");
			return;
		}

		if (m_multiSelect.Size != 1)
		{
			pSelection->Clear();
			return;
		}

		void* iterator = nullptr;
		ImGuiID storageId = 0;

		if (!m_multiSelect.GetNextSelectedItem(&iterator, &storageId))
		{
			pSelection->Clear();
			return;
		}

		for (const HierarchyRow& row : m_rows)
		{
			if (row.id != storageId)
				continue;

			pSelection->Set(row.entity);
			return;
		}

		pSelection->Clear();
	}

	void SceneHierarchyView::CollectSelected(SceneHierarchyContext& context)
	{
		for (const HierarchyRow& row : m_rows)
		{
			if (!m_multiSelect.Contains(row.id))
				continue;

			// context.selectedEntities.PushBack(row.entity);
		}
	}
}