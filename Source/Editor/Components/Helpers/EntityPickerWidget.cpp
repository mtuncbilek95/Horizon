#include "EntityPickerWidget.h"

#include <Editor/Components/Helpers/SearchFilter.h>
#include <Editor/Font/IconsFontAwesome6.h>

#include <Engine/World/ECS/Scene.h>
#include <Engine/World/Components/Tag/NameComponent.h>
#include <Engine/World/Components/EditorOnlyComponent.h>

#include <Runtime/Log/Terminal.h>

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

#include <format>

namespace Horizon::Editor
{
	namespace
	{
		static constexpr const c8* sPickerPopup = "##entityPicker";
		static constexpr const c8* sClearPopup = "##entityClear";
	}

	b8 EntityPickerWidget::Draw(Engine::EntityHandle* pEntity, Engine::Scene* pScene)
	{
		if (pScene == nullptr)
		{
			ImGui::TextDisabled("No active world");
			return false;
		}

		const std::string label = LabelOf(*pEntity, pScene);

		const ImGuiStyle& style = ImGui::GetStyle();
		const f32 pickerWidth = ImGui::GetFrameHeight();
		const f32 dropWidth = ImGui::GetContentRegionAvail().x - pickerWidth - style.ItemInnerSpacing.x;

		b8 changed = false;

		ImGui::Button(label.c_str(), ImVec2(dropWidth, 0.f));

		if (ImGui::BeginPopupContextItem(sClearPopup))
		{
			if (ImGui::MenuItem("Clear"))
			{
				*pEntity = Engine::EntityHandle();
				changed = true;
			}

			ImGui::EndPopup();
		}

		changed |= DrawDropTarget(pEntity, pScene);

		ImGui::SameLine(0.f, style.ItemInnerSpacing.x);

		if (ImGui::Button(ICON_FA_MAGNIFYING_GLASS, ImVec2(pickerWidth, 0.f)))
		{
			m_search.clear();
			ImGui::OpenPopup(sPickerPopup);
		}

		changed |= DrawPicker(pEntity, pScene);

		return changed;
	}

	b8 EntityPickerWidget::DrawDropTarget(Engine::EntityHandle* pEntity, Engine::Scene* pScene)
	{
		if (!ImGui::BeginDragDropTarget())
			return false;

		b8 changed = false;

		if (const ImGuiPayload* pPayload = ImGui::AcceptDragDropPayload(PayloadName, ImGuiDragDropFlags_AcceptBeforeDelivery))
		{
			if (pPayload->DataSize == sizeof(Engine::EntityHandle))
			{
				const Engine::EntityHandle dropped = *static_cast<const Engine::EntityHandle*>(pPayload->Data);
				const b8 acceptable = pScene->GetEntities().IsAlive(dropped);

				if (acceptable && pPayload->IsDelivery())
				{
					*pEntity = dropped;
					changed = true;

					Terminal::Info("EntityPickerWidget", "{} dropped", LabelOf(dropped, pScene));
				}
			}
		}

		ImGui::EndDragDropTarget();
		return changed;
	}

	b8 EntityPickerWidget::DrawPicker(Engine::EntityHandle* pEntity, Engine::Scene* pScene)
	{
		ImGui::SetNextWindowSize(ImVec2(320.f, 360.f), ImGuiCond_Appearing);

		if (!ImGui::BeginPopup(sPickerPopup))
			return false;

		if (ImGui::IsWindowAppearing())
			ImGui::SetKeyboardFocusHere();

		ImGui::SetNextItemWidth(-FLT_MIN);
		ImGui::InputTextWithHint("##search", "Search...", &m_search);
		ImGui::Separator();

		b8 changed = false;

		if (!ImGui::BeginChild("##list", ImVec2(0.f, 0.f), ImGuiChildFlags_None))
		{
			ImGui::EndChild();
			ImGui::EndPopup();
			return false;
		}

		if (m_search.empty())
		{
			if (ImGui::Selectable("None", !pEntity->IsValid()))
			{
				*pEntity = Engine::EntityHandle();
				changed = true;
				ImGui::CloseCurrentPopup();
			}
		}

		pScene->ForEach<Engine::NameComponent>([&](Engine::EntityHandle entt, const Engine::NameComponent&)
			{
				if (pScene->HasComponent<Engine::EditorOnlyComponent>(entt))
					return;

				const std::string label = LabelOf(entt, pScene);

				if (!SearchFilter::Matches(label, m_search))
					return;

				ImGui::PushID(i32(entt.Index()));

				if (ImGui::Selectable(label.c_str(), entt == *pEntity))
				{
					*pEntity = entt;
					changed = true;
					ImGui::CloseCurrentPopup();
				}

				ImGui::PopID();
			});

		ImGui::EndChild();
		ImGui::EndPopup();

		return changed;
	}

	std::string EntityPickerWidget::LabelOf(Engine::EntityHandle entity, Engine::Scene* pScene)
	{
		if (!entity.IsValid())
			return "None";

		if (!pScene->GetEntities().IsAlive(entity))
			return "Missing";

		const Engine::NameComponent* pName = pScene->FindComponent<Engine::NameComponent>(entity);

		if (pName == nullptr || pName->m_name.empty())
			return std::format("Entity {}", entity.Index());

		return pName->m_name;
	}
}