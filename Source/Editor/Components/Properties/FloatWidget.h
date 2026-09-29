#pragma once

#include <Runtime/Math/Scalar.h>
#include <Runtime/RTTR/Reflection.h>
#include <Runtime/RTTR/Attributes/DegreesAttribute.h>
#include <Runtime/RTTR/Attributes/RangeAttribute.h>
#include <Runtime/RTTR/Attributes/SliderAttribute.h>

#include <imgui.h>

namespace Horizon::Editor
{
	struct FloatWidget
	{
		static b8 Draw(const Reflect::Field& field, f32* pValues, i32 count)
		{
			const b8 asDegrees = field.GetCustomAttribute<Reflect::DegreesAttribute>() != nullptr;
			const Reflect::RangeAttribute* pRange = field.GetCustomAttribute<Reflect::RangeAttribute>();
			const b8 asSlider = pRange != nullptr && field.GetCustomAttribute<Reflect::SliderAttribute>() != nullptr;

			if (asDegrees)
			{
				for (i32 i = 0; i < count; i++)
					pValues[i] = Math::RadToDeg(pValues[i]);
			}

			f32 minimum = pRange ? pRange->GetMin() : 0.f;
			f32 maximum = pRange ? pRange->GetMax() : 0.f;

			ImGuiSliderFlags flags = ImGuiSliderFlags_None;

			if (pRange)
				flags |= ImGuiSliderFlags_AlwaysClamp;

			if (count > 1)
				flags |= ImGuiSliderFlags_ColorMarkers;

			ImGui::SetNextItemWidth(-FLT_MIN);

			b8 changed = false;

			if (asSlider)
				changed = ImGui::SliderScalarN("##value", ImGuiDataType_Float, pValues, count, &minimum, &maximum, "%.3f", flags);
			else
				changed = ImGui::DragScalarN("##value", ImGuiDataType_Float, pValues, count, 0.1f, pRange ? &minimum : nullptr, pRange ? &maximum : nullptr, "%.3f", flags);

			if (asDegrees && changed)
			{
				for (i32 i = 0; i < count; i++)
					pValues[i] = Math::DegToRad(pValues[i]);
			}

			return changed;
		}
	};
}
