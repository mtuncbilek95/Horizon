#include "SceneHierarchyView.h"

#include <Editor/Models/SelectionModel.h>
#include <Editor/Renderer/EditorContext.h>
#include <Editor/Domain/DomainFile.h>
#include <Engine/Asset/Scene/SceneSerializer.h>
#include <Engine/Job/JobSystem.h>
#include <Engine/World/WorldService.h>
#include <Engine/World/Components/Tag/NameComponent.h>
#include <Engine/World/Components/EditorOnlyComponent.h>
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
	}

	void SceneHierarchyView::OnInvoke()
	{
		auto* pEngine = GetContext()->pEngine;
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
				GetContext()->pSelection->Clear();
				m_selection.Clear();
				m_renamePath.clear();
				m_renameHandl = Engine::EntityHandle();
				m_currentScene = pWorldService->GetCurrentWorld();
			}
		}

		m_entities.Clear();
		m_currentScene->ForEach<Engine::NameComponent>([&](Engine::EntityHandle entt, const Engine::NameComponent&)
			{
				m_entities.PushBack(entt);
			});

		ImGuiMultiSelectFlags msFlags = ImGuiMultiSelectFlags_ClearOnEscape;

		if (!ImGui::IsPopupOpen(sContextName.data(), ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel))
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

			if (m_currentScene->HasComponent<Engine::EditorOnlyComponent>(entt))
				continue;

			u32 id = u32(entt.Index());
			Engine::NameComponent* nameComp = m_currentScene->FindComponent<Engine::NameComponent>(entt);

			std::string name = ICON_FA_CUBE " ";
			name += nameComp->m_name;

			ImGui::PushID(i32(id));
			ImGui::SetNextItemSelectionUserData(u64(i));
			ImGui::Selectable(name.data(), m_selection.Contains(id));
			ImGui::PopID();
		}

		pMultiIO = ImGui::EndMultiSelect();
		m_selection.ApplyRequests(pMultiIO);

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

		if (sceneContext.selectedEntities.GetCount() == 1)
			GetContext()->pSelection->Set<Engine::EntityTag>(sceneContext.selectedEntities[0]);
		else
			GetContext()->pSelection->Clear();

		m_context.RenderGUI(sContextName.data(), sceneContext);

		if (sceneContext.renameEntity.IsValid())
			BeginRename(sceneContext.renameEntity);

		RenderRenameModal();
	}

	b8 SceneHierarchyView::OnCommand(ViewCommand command)
	{
		if (command != ViewCommand::Save)
			return false;

		SaveSceneToSource();
		return true;
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
		m_renamePath = pNameComp->m_name;

		std::snprintf(m_renameBuffer, sizeof(m_renameBuffer), "%s", pNameComp->m_name.data());
		ImGui::OpenPopup(sPopupName.data());
	}

	void SceneHierarchyView::RenderRenameModal()
	{
		if (!ImGui::BeginPopupModal(sPopupName.data(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
			return;

		b8 accepted = ImGui::InputText("##name", m_renameBuffer, sizeof(m_renameBuffer), ImGuiInputTextFlags_EnterReturnsTrue);

		if (ImGui::Button("OK") || accepted)
		{
			auto* pNameComp = m_currentScene->FindComponent<Engine::NameComponent>(m_renameHandl);
			pNameComp->m_name = m_renameBuffer;
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

	void SceneHierarchyView::SaveSceneToSource()
	{
		auto* pJobSys = GetContext()->pEngine->GetJobSystem();

		pJobSys->SubmitJob(Engine::JobLane::Background, Engine::Job([this]()
			{
				Engine::ReflectionSystem* pReflection = GetContext()->pEngine->GetReflectionSystem();

				JsonArchiveWriter writer;
				Engine::SceneSerializer::Serialize(*m_currentScene, pReflection, writer);

				PAL::FileAccessRequest handle = PAL::File::RequestAccess(GetConnectedFile()->GetSourcePath(),
					PAL::FileOperationAccessPolicy::Write, PAL::FileOperationSharePolicy::SharedWrite);

				PAL::File::WriteString(handle, writer.ToString());

				PAL::File::ReleaseAccess(handle);
			}));
	}
}