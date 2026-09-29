#pragma once

#include <Runtime/Math/Scalar.h>
#include <Runtime/RTTR/Reflection.h>
#include <Runtime/RTTR/Attributes/RangeAttribute.h>
#include <Runtime/RTTR/Attributes/SliderAttribute.h>
#include <Runtime/RTTR/Attributes/StepAttribute.h>

#include <Editor/Components/Helpers/FloatWidget.h>

#include <imgui.h>

namespace Horizon::Editor
{
	template<typename T, ImGuiDataType DataType>
	struct IntegerWidget
	{
		static b8 Draw(const Reflect::Field& field, T* pValue)
		{
			const Reflect::RangeAttribute* pRange = field.GetCustomAttribute<Reflect::RangeAttribute>();
			const Reflect::StepAttribute* pStep = field.GetCustomAttribute<Reflect::StepAttribute>();
			const b8 asSlider = pRange != nullptr && field.GetCustomAttribute<Reflect::SliderAttribute>() != nullptr;

			T minimum = pRange ? static_cast<T>(pRange->GetMin()) : T(0);
			T maximum = pRange ? static_cast<T>(pRange->GetMax()) : T(0);

			ImGuiSliderFlags flags = ImGuiSliderFlags_None;

			if (pRange)
				flags |= ImGuiSliderFlags_AlwaysClamp;

			ImGui::SetNextItemWidth(-FLT_MIN);

			b8 changed = false;

			if (pStep && asSlider)
			{
				const T step = static_cast<T>(pStep->GetStep()) > T(0) ? static_cast<T>(pStep->GetStep()) : T(1);

				changed = ImGui::SliderScalar("##value", DataType, pValue, &minimum, &maximum, nullptr, flags);

				if (changed)
					*pValue = Math::Clamp(static_cast<T>(minimum + ((*pValue - minimum + step / 2) / step) * step), minimum, maximum);

				FloatWidget::DrawTicks(1, static_cast<f32>(minimum), static_cast<f32>(maximum), static_cast<f32>(step));
			}
			else if (pStep)
			{
				T step = static_cast<T>(pStep->GetStep()) > T(0) ? static_cast<T>(pStep->GetStep()) : T(1);
				T fastStep = static_cast<T>(pStep->GetFastStep()) > T(0) ? static_cast<T>(pStep->GetFastStep()) : step * T(10);

				changed = ImGui::InputScalar("##value", DataType, pValue, &step, &fastStep);

				if (changed && pRange)
					*pValue = Math::Clamp(*pValue, minimum, maximum);
			}
			else if (asSlider)
				changed = ImGui::SliderScalar("##value", DataType, pValue, &minimum, &maximum, nullptr, flags);
			else
				changed = ImGui::DragScalar("##value", DataType, pValue, 0.25f, pRange ? &minimum : nullptr, pRange ? &maximum : nullptr, nullptr, flags);

			return changed;
		}
	};
}