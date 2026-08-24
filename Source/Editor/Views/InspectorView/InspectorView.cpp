#include "InspectorView.h"

#include <Engine/Core/Engine.h>
#include <Engine/World/ComponentRegistry.h>
#include <Engine/World/WorldService.h>
#include <Engine/World/ComponentObject.h>
#include <Engine/World/ComponentTypeAttribute.h>

#include <Runtime/Math/Vec3f.h>
#include <Runtime/Math/Quat.h>

namespace Horizon::Editor
{
	namespace
	{
		constexpr ImU32 AxisColorX = IM_COL32(219, 62, 76, 255);
		constexpr ImU32 AxisColorY = IM_COL32(112, 184, 38, 255);
		constexpr ImU32 AxisColorZ = IM_COL32(41, 120, 219, 255);

		b8 DrawAxisField(const c8* pAxis, f32* pValue, ImU32 accentColor, f32 width)
		{
			ImGui::PushID(pAxis);

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

			ImVec2 axisSize = ImGui::CalcTextSize(pAxis);
			ImVec2 axisPos = ImVec2(origin.x + stripWidth + (inset - stripWidth - axisSize.x) * 0.5f,
				origin.y + (height - axisSize.y) * 0.5f);

			pDraw->AddText(axisPos, ImGui::GetColorU32(ImGuiCol_TextDisabled), pAxis);

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

		b8 DrawVectorRow(const c8* pLabel, f32* pValues)
		{
			ImGui::PushID(pLabel);
			ImGui::TableNextRow();

			ImGui::TableSetColumnIndex(0);
			ImGui::AlignTextToFramePadding();
			ImGui::TextUnformatted(pLabel);

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
	}

	void InspectorView::OnInvoke()
	{
		m_activeWorld = GetEngine()->RequestService<Engine::WorldService>()->GetActiveWorld();
	}

	void InspectorView::OnRender()
	{
		if (!m_selectedEntity.IsValid())
			return;

		List<Engine::ComponentObject*> comps;
		m_activeWorld->CollectComponents(m_selectedEntity, comps);

		Engine::ReflectionSystem* pReflection = GetEngine()->GetReflectionSystem();

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
						if (field.GetTypeId() == Reflect::TypeOf<Math::Vec3f>())
						{
							Math::Vec3f& vector = field.GetValueAs<Math::Vec3f>(pComponent);
							f32 values[3] = { vector.X(), vector.Y(), vector.Z() };

							if (DrawVectorRow(field.GetName().c_str(), values))
								vector = Math::Vec3f(values[0], values[1], values[2]);
						}

						if (field.GetTypeId() == Reflect::TypeOf<Math::Quat>())
						{
							Math::Quat& rotation = field.GetValueAs<Math::Quat>(pComponent);
							Math::Vec3f euler = rotation.Euler();
							f32 values[3] = { euler.X(), euler.Y(), euler.Z() };

							if (DrawVectorRow(field.GetName().c_str(), values))
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

	void InspectorView::SetInspectingEntity(Engine::EntityHandle handl)
	{
		m_selectedEntity = handl;
	}
}