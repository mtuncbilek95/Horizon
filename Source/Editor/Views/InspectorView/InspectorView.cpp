#include "InspectorView.h"

#include <Editor/Renderer/EditorContext.h>
#include <Editor/Models/SelectionModel.h>
#include <Editor/Attributes/HideInInspectorAttribute.h>
#include <Editor/Attributes/RangeAttribute.h>
#include <Editor/Attributes/TooltipAttribute.h>

#include <Engine/Core/Engine.h>
#include <Engine/Reflection/ReflectionSystem.h>
#include <Engine/World/ECS/Scene.h>
#include <Engine/World/WorldService.h>
#include <Engine/World/Components/TransformComponent.h>
#include <Engine/World/Components/NameComponent.h>

#include <misc/cpp/imgui_stdlib.h>

namespace Horizon::Editor
{
	namespace
	{
		static constexpr std::string_view sButtonName = "Add Component";
		static constexpr std::string_view sPopupName = "UsableComponents";
	}

	InspectorView::~InspectorView()
	{
		for (auto* pDrawer : m_drawerList)
			Memory::Allocator::Delete(pDrawer);
	}

	void InspectorView::OnInvoke()
	{
		m_worldService = GetContext()->pEngine->RequestService<Engine::WorldService>();
		m_reflSys = GetContext()->pEngine->GetReflectionSystem();

		List<Reflect::Type*> drawerTypes = m_reflSys->GetTypeByBase(Reflect::TypeOf<ComponentDrawer>());
		for (auto* pType : drawerTypes)
		{
			auto* pDrawer = (ComponentDrawer*)pType->Create();
			m_drawerLookups[pDrawer->GetComponentId()] = m_drawerList.GetCount();
			m_drawerList.PushBack(pDrawer);
		}
	}

	void InspectorView::OnRender()
	{
		Engine::EntityHandle entity;
		if (GetContext()->pSelection->Is<Engine::EntityTag>())
			entity = GetContext()->pSelection->Get<Engine::EntityTag>();
		else
			return;

		auto* currScene = m_worldService->GetCurrentWorld();

		const List<Engine::IComponentStorage*>& storages = currScene->GetComponents().GetStorages();

		for (usize i = 0; i < storages.GetCount(); i++)
		{
			Engine::IComponentStorage* pStorage = storages[i];

			if (!pStorage->Contains(entity))
				continue;

			Engine::ComponentTypeId typeId = pStorage->GetComponentTypeId();

			auto it = m_drawerLookups.find(typeId);
			if (it == m_drawerLookups.end())
				continue;

			ComponentDrawer* pDrawer = m_drawerList[it->second];
			pDrawer->m_engine = GetContext()->pEngine;
			pDrawer->m_component = (Engine::ComponentObject*)pStorage->FindRaw(entity);

			Reflect::Type* pType = m_reflSys->GetType(typeId);

			ImGui::PushID((int)i);

			if (ImGui::CollapsingHeader(pType->GetName().c_str(), ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::PushFont(nullptr, ImGui::GetStyle().FontSizeBase * 0.875f);
				ImGui::Indent();
				pDrawer->OnRender();
				ImGui::Unindent();
				ImGui::PopFont();
			}

			ImGui::PopID();
		}

		auto& style = ImGui::GetStyle();
		f32 buttonWidth = ImGui::CalcTextSize(sButtonName.data()).x + style.FramePadding.x * 2.f;
		f32 buttonHeight = ImGui::GetFrameHeight();
		ImVec2 availRegion = ImGui::GetContentRegionAvail();

		f32 offset = (availRegion.x - buttonWidth) * 0.5f;
		if (offset > 0)
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);

		f32 offsetY = availRegion.y - buttonHeight - style.WindowPadding.y;
		if (offsetY > 0)
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + offsetY);

		if (ImGui::Button(sButtonName.data(), ImVec2(buttonWidth, 0.f)))
			ImGui::OpenPopup(sPopupName.data());

		if (!ImGui::BeginPopup(sPopupName.data()))
			return;

		ImGui::InputTextWithHint("##search", "Search...", &m_searchBuffer);
		ImGui::Separator();

		for (usize i = 0; i < storages.GetCount(); i++)
		{
			Engine::IComponentStorage* pStorage = storages[i];
			Reflect::Type* pType = m_reflSys->GetType(pStorage->GetComponentTypeId());

			// If the entity already has the component, make it disabled
			b8 owned = pStorage->Contains(entity);

			if (ImGui::Selectable(pType->GetName().data(), false, owned ? ImGuiSelectableFlags_Disabled : ImGuiSelectableFlags_None))
			{
				auto* pNameComp = currScene->FindComponent<Engine::NameComponent>(entity);
				Terminal::Info(StringOps::GetName(this), "{} has been added to {}", pType->GetName(), pNameComp->m_name.ToString());
				ImGui::CloseCurrentPopup();
			}
		}

		ImGui::EndPopup();
	}
}