#include "SceneHierarchyView.h"

#include <Editor/Renderer/EditorContext.h>
#include <Editor/Components/Helpers/EntityPickerWidget.h>
#include <Editor/Domain/DomainFile.h>
#include <Engine/Asset/Scene/SceneSerializer.h>
#include <Engine/Core/Engine.h>
#include <Engine/World/WorldService.h>
#include <Engine/World/Components/Tag/EntityName.h>
#include <Engine/World/Components/Tag/EditorOnly.h>
#include <Engine/World/ECS/Scene.h>
#include <Runtime/PAL/File/File.h>
#include <Runtime/Serialization/JsonArchive.h>

namespace Horizon::Editor
{
	void SceneHierarchyView::OnInvoke()
	{
		m_context.BootstrapContext(GetContext()->pEngine, "SceneHierarchyView");
		m_world = GetContext()->pEngine->RequestService<Engine::WorldService>();
	}

	void SceneHierarchyView::OnRender(const Engine::EngineFrame& context)
	{
		// First try to get
		if (m_scene != m_world->GetCurrentWorld())
			m_scene = m_world->GetCurrentWorld();

		if (!m_scene)
			return;

		struct EntityEntry
		{
			Engine::EntityHandle handle;
			std::string name;
		};

		List<EntityEntry> currentList;
		m_scene->ForEach<World::Component::EntityName>([&](Engine::EntityHandle handl, World::Component::EntityName& name)
			{
				if (m_scene->HasComponent(handl, Reflect::TypeOf<World::Component::EditorOnly>()))
					return;

				currentList.PushBack({ handl, name.m_name });
			});

		const i32 itemCount = i32(currentList.GetCount());

		m_selection.UserData = &currentList;
		m_selection.AdapterIndexToStorageId = [](ImGuiSelectionBasicStorage* self, int idx)
			{
				List<EntityEntry>* list = static_cast<List<EntityEntry>*>(self->UserData);
				return ImGuiID((*list)[idx].handle.Index());
			};

		ImGuiMultiSelectFlags flags = ImGuiMultiSelectFlags_ClearOnEscape | ImGuiMultiSelectFlags_SelectOnClickRelease;

		if (!ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel))
			flags |= ImGuiMultiSelectFlags_BoxSelect1d | ImGuiMultiSelectFlags_ClearOnClickVoid;

		ImGuiMultiSelectIO* io = ImGui::BeginMultiSelect(flags, m_selection.Size, itemCount);
		m_selection.ApplyRequests(io);

		ImGuiListClipper clipper;
		clipper.Begin(itemCount);
		if (io->RangeSrcItem != -1)
			clipper.IncludeItemByIndex(i32(io->RangeSrcItem));

		while (clipper.Step())
		{
			for (i32 i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
			{
				const EntityEntry& entry = currentList[i];
				const ImGuiID id = ImGuiID(entry.handle.Index());

				ImGui::PushID(id);
				ImGui::SetNextItemSelectionUserData(i);
				std::string name = ICON_FA_CUBE " " + entry.name;
				ImGui::Selectable(name.c_str(), m_selection.Contains(id));

				if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoDisableHover))
				{
					ImGui::SetDragDropPayload(EntityPickerWidget::PayloadName, &entry.handle, sizeof(Engine::EntityHandle));
					ImGui::TextUnformatted(name.c_str());
					ImGui::EndDragDropSource();
				}

				ImGui::PopID();
			}
		}

		io = ImGui::EndMultiSelect();
		m_selection.ApplyRequests(io);

		SceneHierarchyContext menuContext;
		menuContext.pEngine = GetContext()->pEngine;
		menuContext.pCurrentScene = m_scene;

		for (const EntityEntry& entry : currentList)
		{
			if (m_selection.Contains(ImGuiID(entry.handle.Index())))
				menuContext.selectedEntities.PushBack(entry.handle);
		}

		m_context.RenderGUI("SceneHierarchyContextMenu", menuContext);

		if (menuContext.renameEntity.IsValid())
			BeginRename(menuContext.renameEntity);

		RenderRenameModal();

		if (menuContext.selectedEntities.GetCount() == 1)
			GetContext()->pSelection->Set(menuContext.selectedEntities[0]);
		else if (GetContext()->pSelection->Is<Engine::EntityTag>())
			GetContext()->pSelection->Clear();
	}

	b8 SceneHierarchyView::OnCommand(ViewCommand command)
	{
		if (command != ViewCommand::Save)
			return false;

		if (!m_scene || !m_connectedFile)
			return false;

		JsonArchiveWriter writer;
		Asset::SceneSerializer::Serialize(*m_scene, GetContext()->pEngine->GetReflectionSystem(), writer);

		const std::string& path = m_connectedFile->GetSourcePath();

		PAL::FileAccessRequest request = PAL::File::RequestAccess(path, PAL::FileOperationAccessPolicy::Write,
			PAL::FileOperationSharePolicy::Exclusive);

		PAL::File::Truncate(request, 0);

		const b8 wasWritten = PAL::File::WriteString(request, writer.ToString());

		PAL::File::ReleaseAccess(request);

		if (!wasWritten)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} cannot be written", path);
			return false;
		}

		return true;
	}

	void SceneHierarchyView::BeginRename(Engine::EntityHandle entity)
	{
		if (m_renameEntity.IsValid())
			return;

		const auto* pName = m_scene->FindComponent<World::Component::EntityName>(entity);

		if (pName == nullptr)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Entity {} has no EntityName component", entity.Index());
			return;
		}

		m_renameEntity = entity;

		std::snprintf(m_renameBuffer, sizeof(m_renameBuffer), "%s", pName->m_name.c_str());
		ImGui::OpenPopup("Rename - Scene Hierarchy");
	}

	void SceneHierarchyView::RenderRenameModal()
	{
		if (!ImGui::BeginPopupModal("Rename - Scene Hierarchy", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
			return;

		b8 accepted = ImGui::InputText("##name", m_renameBuffer, sizeof(m_renameBuffer), ImGuiInputTextFlags_EnterReturnsTrue);

		if (ImGui::Button("OK") || accepted)
		{
			auto* pName = m_scene->FindComponent<World::Component::EntityName>(m_renameEntity);

			if (pName != nullptr)
				pName->m_name = m_renameBuffer;

			m_renameEntity = {};
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(ImGuiKey_Escape))
		{
			m_renameEntity = {};
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}