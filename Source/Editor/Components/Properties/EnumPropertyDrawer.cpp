#include "EnumPropertyDrawer.h"

#include <Editor/Components/Helpers/FloatWidget.h>
#include <Engine/Reflection/ReflectionSystem.h>
#include <Runtime/RTTR/Attributes/StepAttribute.h>

#include <imgui.h>

#include <string>

namespace Horizon::Editor
{
	b8 EnumPropertyDrawer::OnDraw(const Reflect::Field& field, void* pValue, const PropertyContext& ctx)
	{
		Reflect::Type* pEnumType = ctx.pReflection->GetType(field.GetTypeId());

		if (!pEnumType)
		{
			Terminal::Error(StringOps::GetName(this), "{} enum type is not registered", field.GetName());
			return false;
		}

		ReadOnlyList<const Reflect::EnumValue> values = pEnumType->GetEnumValues();

		if (values.GetCount() == 0)
		{
			ImGui::TextDisabled("empty enum");
			return false;
		}

		const i64 current = ReadUnderlying(pValue, field.GetUnderlyingKind());

		i32 currentIndex = -1;

		for (usize i = 0; i < values.GetCount(); i++)
		{
			if (values[i].value == current)
			{
				currentIndex = static_cast<i32>(i);
				break;
			}
		}

		ImGui::SetNextItemWidth(-FLT_MIN);

		if (field.GetCustomAttribute<Reflect::StepAttribute>())
			return DrawSlider(field, pValue, values, currentIndex);

		return DrawCombo(field, pValue, values, currentIndex);
	}

	b8 EnumPropertyDrawer::DrawSlider(const Reflect::Field& field, void* pValue, ReadOnlyList<const Reflect::EnumValue> values, i32 currentIndex)
	{
		i32 index = currentIndex < 0 ? 0 : currentIndex;
		const i32 last = static_cast<i32>(values.GetCount()) - 1;

		std::string format;

		for (const c8 ch : values[index].name)
		{
			if (ch == '%')
				format.push_back('%');

			format.push_back(ch);
		}

		const b8 changed = ImGui::SliderInt("##value", &index, 0, last, format.c_str(), ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_NoInput);

		FloatWidget::DrawTicks(1, 0.f, static_cast<f32>(last), 1.f);

		if (!changed && currentIndex >= 0)
			return false;

		WriteUnderlying(pValue, field.GetUnderlyingKind(), values[index].value);
		return true;
	}

	b8 EnumPropertyDrawer::DrawCombo(const Reflect::Field& field, void* pValue, ReadOnlyList<const Reflect::EnumValue> values, i32 currentIndex)
	{
		const c8* pPreview = currentIndex < 0 ? "Unknown" : values[currentIndex].name.c_str();

		if (!ImGui::BeginCombo("##value", pPreview))
			return false;

		b8 changed = false;

		for (usize i = 0; i < values.GetCount(); i++)
		{
			const b8 selected = static_cast<i32>(i) == currentIndex;

			if (ImGui::Selectable(values[i].name.c_str(), selected))
			{
				WriteUnderlying(pValue, field.GetUnderlyingKind(), values[i].value);
				changed = true;
			}

			if (selected)
				ImGui::SetItemDefaultFocus();
		}

		ImGui::EndCombo();
		return changed;
	}

	i64 EnumPropertyDrawer::ReadUnderlying(const void* pValue, Reflect::TypeKind kind)
	{
		switch (kind)
		{
		case Reflect::TypeKind::Char:
			return *static_cast<const c8*>(pValue);
		case Reflect::TypeKind::Signed8:
			return *static_cast<const i8*>(pValue);
		case Reflect::TypeKind::Signed16:
			return *static_cast<const i16*>(pValue);
		case Reflect::TypeKind::Signed32:
			return *static_cast<const i32*>(pValue);
		case Reflect::TypeKind::Signed64:
			return *static_cast<const i64*>(pValue);
		case Reflect::TypeKind::Unsigned8:
			return *static_cast<const u8*>(pValue);
		case Reflect::TypeKind::Unsigned16:
			return *static_cast<const u16*>(pValue);
		case Reflect::TypeKind::Unsigned32:
			return *static_cast<const u32*>(pValue);
		case Reflect::TypeKind::Unsigned64:
			return static_cast<i64>(*static_cast<const u64*>(pValue));
		default:
			Terminal::Error("EnumPropertyDrawer", "Kind is not an enum underlying type, reading 0");
			return 0;
		}
	}

	void EnumPropertyDrawer::WriteUnderlying(void* pValue, Reflect::TypeKind kind, i64 value)
	{
		switch (kind)
		{
		case Reflect::TypeKind::Char:
			*static_cast<c8*>(pValue) = static_cast<c8>(value);
			break;
		case Reflect::TypeKind::Signed8:
			*static_cast<i8*>(pValue) = static_cast<i8>(value);
			break;
		case Reflect::TypeKind::Signed16:
			*static_cast<i16*>(pValue) = static_cast<i16>(value);
			break;
		case Reflect::TypeKind::Signed32:
			*static_cast<i32*>(pValue) = static_cast<i32>(value);
			break;
		case Reflect::TypeKind::Signed64:
			*static_cast<i64*>(pValue) = value;
			break;
		case Reflect::TypeKind::Unsigned8:
			*static_cast<u8*>(pValue) = static_cast<u8>(value);
			break;
		case Reflect::TypeKind::Unsigned16:
			*static_cast<u16*>(pValue) = static_cast<u16>(value);
			break;
		case Reflect::TypeKind::Unsigned32:
			*static_cast<u32*>(pValue) = static_cast<u32>(value);
			break;
		case Reflect::TypeKind::Unsigned64:
			*static_cast<u64*>(pValue) = static_cast<u64>(value);
			break;
		default:
			Terminal::Error("EnumPropertyDrawer", "Kind is not an enum underlying type, nothing written");
			break;
		}
	}
}