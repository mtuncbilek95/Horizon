#include "InspectorView.h"

#include <Editor/Renderer/EditorContext.h>
#include <Editor/Models/SelectionModel.h>

#include <Engine/Core/Engine.h>
#include <Engine/Reflection/ReflectionSystem.h>
#include <Engine/World/ComponentTypeAttribute.h>
#include <Engine/World/ComponentObject.h>
#include <Engine/World/WorldService.h>

#include <Runtime/Containers/Guid.h>
#include <Runtime/Math/Vec3f.h>
#include <Runtime/Math/Quat.h>

namespace Horizon::Editor
{
	namespace
	{
		constexpr u32 AxisColorX = IM_COL32(219, 62, 76, 255);
		constexpr u32 AxisColorY = IM_COL32(112, 184, 38, 255);
		constexpr u32 AxisColorZ = IM_COL32(41, 120, 219, 255);
	}

	void InspectorView::OnInvoke()
	{
		m_currentWorld = GetContext()->pEngine->RequestService<Engine::WorldService>()->GetActiveWorld();
	}

	void InspectorView::OnRender()
	{
		m_selected = GetContext()->pSelection->Get<Engine::EntityTag>();

		if (!m_currentWorld || !m_selected.IsValid())
			return;

		const ImGuiStyle& style = ImGui::GetStyle();

		const f32 kButtonHeight = 30.0f;
		const f32 kBottomHeight = kButtonHeight + style.ItemSpacing.y * 2.0f + 1.0f;

		// Components, full area of the rest. It will have its own scroller
		ImGui::BeginGroup();
		{
			List<Engine::ComponentObject*> comps;
			m_currentWorld->CollectComponents(m_selected, comps);

			Engine::ReflectionSystem* pReflection = GetContext()->pEngine->GetReflectionSystem();

			for (Engine::ComponentObject* pComponent : comps)
			{
				Reflect::Type* pType = pReflection->GetType(pComponent->GetTypeId());

				if (!pType)
					continue;

				const c8* pHeader = pType->GetName().c_str();
				auto* pAttribute = pType->GetCustomAttribute<Engine::ComponentTypeAttribute>();

				if (pAttribute)
					pHeader = pAttribute->GetComponentName().c_str();

				ImGui::PushID((i32)pComponent->GetTypeId().Index());

				if (ImGui::CollapsingHeader(pHeader, ImGuiTreeNodeFlags_DefaultOpen))
				{
					ImGui::Spacing();

					if (ImGui::BeginTable("##fields", 2, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_PadOuterX))
					{
						ImGui::TableSetupColumn("##label", ImGuiTableColumnFlags_WidthFixed, 90.0f);
						ImGui::TableSetupColumn("##value", ImGuiTableColumnFlags_WidthStretch);

						for (const Reflect::Field& field : pType->GetFields())
						{
							if (field.GetTypeId() == Reflect::TypeOf<Guid>())
							{
								Guid& guid = field.GetValueAs<Guid>(pComponent);
								//DrawGuid(field.GetName().c_str(), guid);
							}

							if (field.GetTypeId() == Reflect::TypeOf<Math::Vec3f>())
							{
								Math::Vec3f& vector = field.GetValueAs<Math::Vec3f>(pComponent);
								f32 values[3] = { vector.X(), vector.Y(), vector.Z() };

								if (DrawVec3(field.GetName().c_str(), values))
									vector = Math::Vec3f(values[0], values[1], values[2]);
							}

							if (field.GetTypeId() == Reflect::TypeOf<Math::Quat>())
							{
								Math::Quat& rotation = field.GetValueAs<Math::Quat>(pComponent);
								Math::Vec3f euler = rotation.Euler();
								f32 values[3] = { euler.X(), euler.Y(), euler.Z() };

								if (DrawVec3(field.GetName().c_str(), values))
									rotation = Math::Quat::MakeFromEuler(Math::Vec3f(values[0], values[1], values[2]));
							}
						}

						ImGui::EndTable();
					}

					ImGui::Spacing();
				}

				ImGui::PopID();
			}

		}
		ImGui::EndGroup();

		ImGui::Separator();

		// AddComponent & behaviour
		ImGui::BeginGroup();
		{
			f32 width = ImGui::GetContentRegionAvail().x;
			f32 buttonWidth = width * 0.65f;

			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (width - buttonWidth) * 0.5f);
			if (ImGui::Button(ICON_FA_PLUS " Add Component", ImVec2(buttonWidth, kButtonHeight)))
			{
				m_PopupPosX = ImGui::GetItemRectMin().x;
				m_PopupPosY = ImGui::GetItemRectMax().y + 4.0f;
				m_PopupWidth = buttonWidth;

				m_FocusSearch = true;

				ImGui::OpenPopup("##AddComponentPopup");
			}

			DrawAddComponentPopup();
		}
		ImGui::EndGroup();
	}

	void InspectorView::DrawAddComponentPopup()
	{
		ImGui::SetNextWindowPos(ImVec2(m_PopupPosX, m_PopupPosY));
		ImGui::SetNextWindowSize(ImVec2(m_PopupWidth, 380.0f));

		const b8 open = ImGui::BeginPopup("##AddComponentPopup");
		if (!open)
			return;

		ImGui::BeginChild("##ComponentList", ImVec2(0, 0), false);
		{
			auto* pReflect = GetContext()->pEngine->GetReflectionSystem();

			List<Reflect::Type*> types = pReflect->GetTypeByBase(Reflect::TypeOf<Engine::ComponentObject>());
			for (auto* type : types)
			{
				auto* pAttr = type->GetCustomAttribute<Engine::ComponentTypeAttribute>();
				if (!pAttr->GetVisibleOnSystem())
					continue;

				if (ImGui::Selectable(pAttr->GetComponentName().c_str(), false, ImGuiSelectableFlags_AllowDoubleClick))
				{
					if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
					{
						Terminal::Warn(StringOps::GetName(this), "{} has been selected to be added!", pAttr->GetComponentName());
						ImGui::CloseCurrentPopup();
					}
				}
			}
		}

		ImGui::EndChild();
		ImGui::EndPopup();
	}

	b8 InspectorView::DrawVec3(const std::string& label, f32* pValues)
	{
		ImGui::PushID(label.c_str());
		ImGui::TableNextRow();

		ImGui::TableSetColumnIndex(0);
		ImGui::AlignTextToFramePadding();
		ImGui::TextUnformatted(label.c_str());

		ImGui::TableSetColumnIndex(1);

		const f32 spacing = 6.0f;
		f32 available = ImGui::GetContentRegionAvail().x;
		f32 fieldWidth = (available - spacing * 2.0f) / 3.0f;

		b8 changed = false;

		changed |= DrawAxisField("X", &pValues[0], AxisColorX, fieldWidth);
		ImGui::SameLine(0.0f, spacing);
		changed |= DrawAxisField("Y", &pValues[1], AxisColorY, fieldWidth);
		ImGui::SameLine(0.0f, spacing);
		changed |= DrawAxisField("Z", &pValues[2], AxisColorZ, fieldWidth);

		ImGui::PopID();

		return changed;
	}

	b8 InspectorView::DrawAxisField(const std::string& axis, f32* pValue, u32 accentColor, f32 width)
	{
		ImGui::PushID(axis.c_str());

		ImDrawList* pDraw = ImGui::GetWindowDrawList();
		ImVec2 origin = ImGui::GetCursorScreenPos();

		const f32 height = ImGui::GetFrameHeight();
		const f32 rounding = ImGui::GetStyle().FrameRounding;
		const f32 stripWidth = 3.0f;
		const f32 inset = height * 0.85f;

		ImVec2 corner = ImVec2(origin.x + width, origin.y + height);
		pDraw->AddRectFilled(origin, corner, ImGui::GetColorU32(ImGuiCol_FrameBg), rounding);

		pDraw->PushClipRect(origin, ImVec2(origin.x + stripWidth, corner.y), true);
		pDraw->AddRectFilled(origin, corner, accentColor, rounding);
		pDraw->PopClipRect();

		ImVec2 axisSize = ImGui::CalcTextSize(axis.c_str());
		ImVec2 axisPos = ImVec2(origin.x + stripWidth + (inset - stripWidth - axisSize.x) * 0.5f,
			origin.y + (height - axisSize.y) * 0.5f);

		pDraw->AddText(axisPos, ImGui::GetColorU32(ImGuiCol_TextDisabled), axis.c_str());

		ImGui::SetCursorScreenPos(ImVec2(origin.x + inset, origin.y));
		ImGui::SetNextItemWidth(width - inset);

		ImGui::PushStyleColor(ImGuiCol_FrameBg, 0);
		ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, 0);
		ImGui::PushStyleColor(ImGuiCol_FrameBgActive, 0);

		b8 changed = ImGui::DragFloat("##value", pValue, 0.01f, 0.0f, 0.0f, "%.3f");

		ImGui::PopStyleColor(3);

		if (ImGui::IsItemActive() || ImGui::IsItemHovered())
			pDraw->AddRect(origin, corner, accentColor, rounding, 0, 1.0f);

		ImGui::PopID();

		return changed;
	}

}