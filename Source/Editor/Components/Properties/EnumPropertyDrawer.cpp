#include "EnumPropertyDrawer.h"

#include <Engine/Reflection/ReflectionSystem.h>
#include <imgui.h>

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

		i64 current = ReadUnderlying(pValue, field.GetUnderlyingKind());
		const c8* pPreview = "Unknown";

		for (const Reflect::EnumValue& value : pEnumType->GetEnumValues())
		{
			if (value.value == current)
			{
				pPreview = value.name.c_str();
				break;
			}
		}

		b8 changed = false;
		ImGui::SetNextItemWidth(-FLT_MIN);

		if (!ImGui::BeginCombo("##value", pPreview))
			return false;

		for (const Reflect::EnumValue& value : pEnumType->GetEnumValues())
		{
			const b8 selected = value.value == current;

			if (ImGui::Selectable(value.name.c_str(), selected))
			{
				WriteUnderlying(pValue, field.GetUnderlyingKind(), value.value);
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
