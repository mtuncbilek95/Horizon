#include "SceneHierarchyView.h"

#include <Editor/Models/SelectionModel.h>
#include <Editor/Renderer/EditorContext.h>

#include <Engine/World/WorldService.h>
#include <Engine/World/Components/NameComponent.h>

#include <Runtime/Containers/StringOps.h>
#include <Runtime/Log/Terminal.h>

namespace Horizon::Editor
{
	void SceneHierarchyView::OnInvoke()
	{
		auto* pEngine = GetContext()->pEngine;
		auto* pWorldService = pEngine->RequestService<Engine::WorldService>();
		m_currentScene = pWorldService->GetCurrentWorld();

		m_context.BootstrapContext(GetContext()->pEngine, "SceneHierarchyView");
	}

	void SceneHierarchyView::OnRender()
	{
		m_entities.Clear();
		m_currentScene->ForEach<Engine::NameComponent>([&](Engine::EntityHandle entt, const Engine::NameComponent&)
			{
				m_entities.PushBack(entt);
			});

		ImGuiMultiSelectFlags msFlags = ImGuiMultiSelectFlags_ClearOnEscape;

		if (!ImGui::IsPopupOpen("SceneHierarchyContext", ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel))
			msFlags |= ImGuiMultiSelectFlags_BoxSelect1d | ImGuiMultiSelectFlags_ClearOnClickVoid;

		ImGuiMultiSelectIO* pMultiIO = ImGui::BeginMultiSelect(msFlags, m_selection.Size, static_cast<i32>(m_entities.GetCount()));

		m_selection.UserData = this;
		m_selection.AdapterIndexToStorageId = [](ImGuiSelectionBasicStorage* pSelf, int index)
			{
				SceneHierarchyView* pView = static_cast<SceneHierarchyView*>(pSelf->UserData);
				return static_cast<ImGuiID>(pView->m_entities[static_cast<usize>(index)].Index());
			};

		m_selection.ApplyRequests(pMultiIO);

		for (usize i = 0; i < m_entities.GetCount(); i++)
		{
			const Engine::EntityHandle entt = m_entities[i];
			u32 id = u32(entt.Index());
			Engine::NameComponent* nameComp = m_currentScene->FindComponent<Engine::NameComponent>(entt);

			std::string name = ICON_FA_CUBE " ";
			name += nameComp->m_name.ToString();

			ImGui::PushID(i32(id));
			ImGui::SetNextItemSelectionUserData(u64(i));
			ImGui::Selectable(name.data(), m_selection.Contains(id));
			ImGui::PopID();
		}

		pMultiIO = ImGui::EndMultiSelect();
		m_selection.ApplyRequests(pMultiIO);

		SceneHierarchyContext context = {};
		context.pEngine = GetContext()->pEngine;
		context.pCurrentScene = m_currentScene;

		void* pIt = nullptr;
		ImGuiID selectedId = 0;

		while (m_selection.GetNextSelectedItem(&pIt, &selectedId))
		{
			const Engine::EntityHandle entt = m_currentScene->GetEntities().GetHandleAt(selectedId);

			if (entt.IsValid())
				context.selectedEntities.PushBack(entt);
		}

		if (context.selectedEntities.GetCount() == 1)
			GetContext()->pSelection->Set<Engine::EntityTag>(context.selectedEntities[0]);
		else
			GetContext()->pSelection->Clear();

		m_context.RenderGUI("SceneHierarchyContext", context);

		if (context.renameEntity.IsValid())
			BeginRename(context.renameEntity);

		RenderRenameModal();
	}

	void SceneHierarchyView::BeginRename(Engine::EntityHandle handl)
	{
		if (!m_renamePath.empty())
			return;

		// TODO: CurrScene will be gone later?
		if (!m_currentScene)
			return;

		auto* pNameComp = m_currentScene->FindComponent<Engine::NameComponent>(handl);
		m_renameHandl = handl;
		m_renamePath = pNameComp->m_name.ToString();

		std::snprintf(m_renameBuffer, sizeof(m_renameBuffer), "%s", pNameComp->m_name.ToString().data());
		ImGui::OpenPopup("Rename - Scene Hierarchy");
	}

	void SceneHierarchyView::RenderRenameModal()
	{
		if (!ImGui::BeginPopupModal("Rename - Scene Hierarchy", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
			return;

		b8 accepted = ImGui::InputText("##name", m_renameBuffer, sizeof(m_renameBuffer), ImGuiInputTextFlags_EnterReturnsTrue);

		if (ImGui::Button("OK") || accepted)
		{
			auto* pNameComp = m_currentScene->FindComponent<Engine::NameComponent>(m_renameHandl);
			pNameComp->m_name = NameId(m_renameBuffer);
			m_renamePath.clear();
			m_renameHandl = Engine::EntityHandle();
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(ImGuiKey_Escape))
		{
			m_renamePath.clear();
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

}