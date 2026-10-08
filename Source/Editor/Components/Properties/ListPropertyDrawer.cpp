#include "ListPropertyDrawer.h"

#include <Editor/Components/PropertyRenderer.h>
#include <Editor/Font/IconsFontAwesome6.h>
#include <Editor/Renderer/Utils/ImGuiUtils.h>

#include <Engine/Reflection/ReflectionSystem.h>

#include <Runtime/RTTR/Attributes/ReadOnlyAttribute.h>
#include <Runtime/RTTR/Attributes/TooltipAttribute.h>

#include <imgui.h>

namespace Horizon::Editor
{
	namespace
	{
		static constexpr const c8* sClearPopup = "##listClear";
	}

	b8 ListPropertyDrawer::OnDraw(const Reflect::Field& field, void* pValue, const PropertyContext& ctx)
	{
		ListBase* pList = static_cast<ListBase*>(pValue);

		PropertyDrawer* pElementDrawer = nullptr;
		const Reflect::Type* pElementType = nullptr;

		if (field.GetKind() == Reflect::TypeKind::Enum)
			pElementDrawer = ctx.pRenderer->FindDrawer(Reflect::TypeOf<Reflect::EnumValue>());
		else
			pElementDrawer = ctx.pRenderer->FindDrawer(field.GetTypeId());

		if (pElementDrawer == nullptr && field.GetKind() == Reflect::TypeKind::Object)
			pElementType = ctx.pReflection->GetType(field.GetTypeId());

		const b8 readOnly = field.GetCustomAttribute<Reflect::ReadOnlyAttribute>() != nullptr;

		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::PushID(field.GetName().c_str());

		const b8 open = ImGui::TreeNodeEx(PropertyRenderer::ToDisplayLabel(field.GetName()).c_str(), ImGuiTreeNodeFlags_DefaultOpen);

		if (const auto* pTooltip = field.GetCustomAttribute<Reflect::TooltipAttribute>())
		{
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("%s", pTooltip->GetTooltip().c_str());
		}

		ImGui::TableNextColumn();

		b8 changed = DrawHeader(pList, readOnly);

		if (open)
		{
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::TableNextColumn();

			changed |= DrawElements(field, pList, pElementDrawer, pElementType, ctx, readOnly);
			ImGui::TreePop();
		}

		ImGui::PopID();
		return changed;
	}

	b8 ListPropertyDrawer::DrawHeader(ListBase* pList, b8 readOnly)
	{
		const ImGuiStyle& style = ImGui::GetStyle();
		const f32 buttonWidth = ImGui::GetFrameHeight();

		b8 changed = false;

		if (readOnly)
			ImGui::BeginDisabled();

		if (ImGui::Button(ICON_FA_PLUS, ImVec2(buttonWidth, 0.f)))
		{
			const usize before = pList->GetCount();
			pList->Resize(before + 1);
			changed = pList->GetCount() != before;
		}

		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Add element");

		ImGui::SameLine(0.f, style.ItemInnerSpacing.x);

		if (ImGui::Button(ICON_FA_TRASH_CAN, ImVec2(buttonWidth, 0.f)))
			ImGui::OpenPopup(sClearPopup);

		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Clear all");

		if (ImGui::BeginPopup(sClearPopup))
		{
			if (ImGui::MenuItem("Clear all"))
			{
				pList->Resize(0);
				changed = true;
			}

			ImGui::EndPopup();
		}

		if (readOnly)
			ImGui::EndDisabled();

		ImGui::SameLine(0.f, style.ItemInnerSpacing.x);
		ImGui::AlignTextToFramePadding();
		ImGui::TextDisabled("%zu items", pList->GetCount());

		return changed;
	}

	b8 ListPropertyDrawer::DrawElements(const Reflect::Field& field, ListBase* pList, PropertyDrawer* pElementDrawer, const Reflect::Type* pElementType, const PropertyContext& ctx, b8 readOnly)
	{
		b8 changed = false;
		i64 removeIndex = -1;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(6.0f, 6.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(6.0f, 2.0f));
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGuiUtils::Hex("#1C1C1C"));

		const b8 visible = ImGui::BeginChild("##elements", ImVec2(0.f, 0.f), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY);

		if (visible)
		{
			if (pList->GetCount() == 0)
			{
				ImGui::TextDisabled("Empty");
			}
			else if (ImGui::BeginTable("elements", 2, ImGuiTableFlags_SizingFixedFit))
			{
				ImGui::TableSetupColumn("label", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);

				for (usize i = 0; i < pList->GetCount(); i++)
					changed |= DrawElementRow(field, pList, i, pElementDrawer, pElementType, ctx, readOnly, removeIndex);

				ImGui::EndTable();
			}
		}

		ImGui::EndChild();

		ImGui::PopStyleColor();
		ImGui::PopStyleVar(2);

		if (removeIndex >= 0)
		{
			pList->RemoveAt(static_cast<usize>(removeIndex));
			changed = true;
		}

		return changed;
	}

	b8 ListPropertyDrawer::DrawElementRow(const Reflect::Field& field, ListBase* pList, usize index, PropertyDrawer* pElementDrawer, const Reflect::Type* pElementType, 
		const PropertyContext& ctx, b8 readOnly, i64& removeIndex)
	{
		const ImGuiStyle& style = ImGui::GetStyle();
		const f32 buttonWidth = ImGui::GetFrameHeight();

		void* pElement = pList->GetElementAt(index);
		b8 changed = false;

		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::PushID(static_cast<i32>(index));

		b8 nestedOpen = false;

		if (pElementType != nullptr)
		{
			nestedOpen = ImGui::TreeNodeEx("##element", ImGuiTreeNodeFlags_DefaultOpen, "Element %zu", index);
		}
		else
		{
			ImGui::AlignTextToFramePadding();
			ImGui::Text("Element %zu", index);
		}

		ImGui::TableNextColumn();

		if (readOnly)
			ImGui::BeginDisabled();

		if (ImGui::Button(ICON_FA_XMARK, ImVec2(buttonWidth, 0.f)))
			removeIndex = static_cast<i64>(index);

		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Remove element");

		if (pElementDrawer != nullptr)
		{
			ImGui::SameLine(0.f, style.ItemInnerSpacing.x);
			changed |= pElementDrawer->OnDraw(field, pElement, ctx);
		}
		else if (pElementType == nullptr)
		{
			ImGui::SameLine(0.f, style.ItemInnerSpacing.x);
			ImGui::AlignTextToFramePadding();
			ImGui::TextDisabled("no drawer");
		}

		if (readOnly)
			ImGui::EndDisabled();

		if (nestedOpen)
		{
			ctx.pRenderer->DrawFields(*pElementType, pElement);
			ImGui::TreePop();
		}

		ImGui::PopID();
		return changed;
	}
}