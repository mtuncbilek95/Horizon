#include "InspectorView.h"

#include <Editor/Renderer/EditorContext.h>
#include <Editor/Models/SelectionModel.h>
#include <Engine/Core/Engine.h>
#include <Engine/Reflection/ReflectionSystem.h>
#include <Engine/World/ECS/Scene.h>
#include <Engine/World/ECS/ComponentIdAttribute.h>
#include <Engine/World/WorldService.h>
#include <Engine/World/Components/Tag/NameComponent.h>
#include <Engine/World/Components/EditorOnlyComponent.h>

#include <misc/cpp/imgui_stdlib.h>

namespace Horizon::Editor
{
	namespace
	{
		static constexpr std::string_view sButtonName = "Add Component";
		static constexpr std::string_view sPopupName = "UsableComponents";
	}

	void InspectorView::OnInvoke()
	{
		m_worldService = GetContext()->pEngine->RequestService<Engine::WorldService>();
		m_reflSys = GetContext()->pEngine->GetReflectionSystem();

		m_properties.Initialize(GetContext()->pEngine);
	}

	void InspectorView::OnRender(const Engine::EngineFrame& context)
	{
		Engine::EntityHandle entity;
		if (GetContext()->pSelection->Is<Engine::EntityTag>())
			entity = GetContext()->pSelection->Get<Engine::EntityTag>();
		else
			return;

		auto* currScene = m_worldService->GetCurrentWorld();

		const List<Engine::ComponentStorage*>& storages = currScene->GetComponents().GetStorages();

		for (usize i = 0; i < storages.GetCount(); i++)
		{
			Engine::ComponentStorage* pStorage = storages[i];

			if (!pStorage->Contains(entity))
				continue;

			Engine::ComponentTypeId typeId = pStorage->GetComponentTypeId();

			if (typeId == Reflect::TypeOf<Engine::NameComponent>())
				continue;

			Reflect::Type* pType = m_reflSys->GetType(typeId);

			if (!pType)
			{
				Terminal::Error(StringOps::GetName(this), "Component storage has an unregistered type");
				continue;
			}

			const auto* pId = pType->GetCustomAttribute<Engine::ComponentIdAttribute>();
			const std::string& header = pId ? pId->GetDisplayName() : pType->GetName();

			ImGui::PushID((i32)i);

			b8 keepComponent = true;

			ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
			b8 collapsingHead = ImGui::CollapsingHeader(header.c_str(), &keepComponent, ImGuiTreeNodeFlags_DefaultOpen);
			ImGui::PopFont();

			if (!keepComponent)
			{
				currScene->RemoveComponent(entity, typeId);
				ImGui::PopID();
				break;
			}

			if (collapsingHead)
				m_properties.DrawObject(*pType, pStorage->Find(entity));

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

		List<Reflect::Type*> compTypes = m_reflSys->GetTypeByBase(Reflect::TypeOf<Engine::ComponentObject>());
		for (usize i = 0; i < compTypes.GetCount(); i++)
		{
			Reflect::Type* pCompType = compTypes[i];

			if (pCompType->GetIsAbstract())
				continue;

			if (pCompType->GetTypeId() == Reflect::TypeOf<Engine::NameComponent>())
				continue;

			if (pCompType->GetTypeId() == Reflect::TypeOf<Engine::EditorOnlyComponent>())
				continue;

			const auto* pId = pCompType->GetCustomAttribute<Engine::ComponentIdAttribute>();
			const std::string& label = pId ? pId->GetDisplayName() : pCompType->GetName();

			if (!m_searchBuffer.empty() && label.find(m_searchBuffer) == std::string::npos)
				continue;

			b8 ownedByEntt = currScene->HasComponent(entity, pCompType->GetTypeId());

			if (ImGui::Selectable(label.c_str(), false, ownedByEntt ? ImGuiSelectableFlags_Disabled : ImGuiSelectableFlags_None))
			{
				currScene->AddComponent(entity, pCompType->GetTypeId());

				auto* pNameComp = currScene->FindComponent<Engine::NameComponent>(entity);
				Terminal::Info(StringOps::GetName(this), "{} has been added to {}", label, pNameComp->m_name);
				ImGui::CloseCurrentPopup();
			}
		}

		ImGui::EndPopup();
	}
}