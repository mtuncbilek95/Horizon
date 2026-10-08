#include "StringPropertyDrawer.h"

#include <Runtime/RTTR/Attributes/BlobAttribute.h>
#include <Runtime/RTTR/Attributes/RangeAttribute.h>
#include <Runtime/RTTR/Attributes/ReadOnlyAttribute.h>

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

namespace Horizon::Editor
{
	namespace
	{
		static constexpr f32 sMultilineRows = 4.f;

		struct LengthLimit
		{
			i32 max = 0;
		};

		static i32 LimitLength(ImGuiInputTextCallbackData* pData)
		{
			auto* pLimit = static_cast<LengthLimit*>(pData->UserData);

			if (pData->EventFlag == ImGuiInputTextFlags_CallbackEdit && pData->BufTextLen > pLimit->max)
				pData->DeleteChars(pLimit->max, pData->BufTextLen - pLimit->max);

			return 0;
		}
	}

	b8 StringPropertyDrawer::OnDraw(const Reflect::Field& field, void* pValue, const PropertyContext& ctx)
	{
		std::string* pString = static_cast<std::string*>(pValue);

		const Reflect::RangeAttribute* pRange = field.GetCustomAttribute<Reflect::RangeAttribute>();
		const b8 readOnly = field.GetCustomAttribute<Reflect::ReadOnlyAttribute>() != nullptr;
		const b8 multiline = field.GetCustomAttribute<Reflect::BlobAttribute>() != nullptr;

		ImGuiInputTextFlags flags = ImGuiInputTextFlags_None;
		ImGuiInputTextCallback callback = nullptr;
		LengthLimit limit;

		if (readOnly)
			flags |= ImGuiInputTextFlags_ReadOnly;

		if (pRange && pRange->GetMax() > 0.f)
		{
			limit.max = static_cast<i32>(pRange->GetMax());
			flags |= ImGuiInputTextFlags_CallbackEdit;
			callback = LimitLength;
		}

		b8 changed = false;

		if (multiline)
		{
			const ImGuiStyle& style = ImGui::GetStyle();
			const f32 height = ImGui::GetTextLineHeight() * sMultilineRows + style.FramePadding.y * 2.f;

			changed = ImGui::InputTextMultiline("##value", pString, ImVec2(-FLT_MIN, height), flags, callback, &limit);
		}
		else
		{
			ImGui::SetNextItemWidth(-FLT_MIN);
			changed = ImGui::InputText("##value", pString, flags, callback, &limit);
		}

		if (limit.max > 0 && ImGui::IsItemHovered())
			ImGui::SetTooltip("%zu / %d", pString->size(), limit.max);

		return changed;
	}
}