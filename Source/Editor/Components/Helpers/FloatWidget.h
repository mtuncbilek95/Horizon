#pragma once

#include <Runtime/Math/Scalar.h>
#include <Runtime/RTTR/Reflection.h>
#include <Runtime/RTTR/Attributes/DegreesAttribute.h>
#include <Runtime/RTTR/Attributes/RangeAttribute.h>
#include <Runtime/RTTR/Attributes/SliderAttribute.h>
#include <Runtime/RTTR/Attributes/StepAttribute.h>

#include <imgui.h>

#include <cmath>

namespace Horizon::Editor
{
	struct FloatWidget
	{
		static b8 Draw(const Reflect::Field& field, f32* pValues, i32 count)
		{
			const b8 asDegrees = field.GetCustomAttribute<Reflect::DegreesAttribute>() != nullptr;
			const Reflect::RangeAttribute* pRange = field.GetCustomAttribute<Reflect::RangeAttribute>();
			const Reflect::StepAttribute* pStep = field.GetCustomAttribute<Reflect::StepAttribute>();
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

			if (pStep && asSlider)
			{
				const f32 step = pStep->GetStep();

				changed = ImGui::SliderScalarN("##value", ImGuiDataType_Float, pValues, count, &minimum, &maximum, "%.3f", flags);

				if (changed)
				{
					for (i32 i = 0; i < count; i++)
						pValues[i] = Math::Clamp(minimum + std::round((pValues[i] - minimum) / step) * step, minimum, maximum);
				}

				DrawTicks(count, minimum, maximum, step);
			}
			else if (pStep)
			{
				f32 step = pStep->GetStep();
				f32 fastStep = pStep->GetFastStep();

				changed = ImGui::InputScalarN("##value", ImGuiDataType_Float, pValues, count, &step, &fastStep, "%.3f");

				if (changed && pRange)
				{
					for (i32 i = 0; i < count; i++)
						pValues[i] = Math::Clamp(pValues[i], minimum, maximum);
				}
			}
			else if (asSlider)
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

		static void DrawTicks(i32 count, f32 minimum, f32 maximum, f32 step)
		{
			if (step <= 0.f || maximum <= minimum)
				return;

			const i32 tickCount = static_cast<i32>((maximum - minimum) / step);

			if (tickCount < 1 || tickCount > 64)
				return;

			ImDrawList* pDraw = ImGui::GetWindowDrawList();
			const ImGuiStyle& style = ImGui::GetStyle();
			const ImVec2 itemMin = ImGui::GetItemRectMin();
			const ImVec2 itemMax = ImGui::GetItemRectMax();
			const ImU32 color = ImGui::GetColorU32(ImGuiCol_TextDisabled);

			const f32 totalWidth = itemMax.x - itemMin.x;
			const f32 componentWidth = (totalWidth - style.ItemInnerSpacing.x * (count - 1)) / count;
			const f32 grabHalf = style.GrabMinSize * 0.5f;
			const f32 tickTop = itemMax.y - 4.f;

			for (i32 c = 0; c < count; c++)
			{
				const f32 left = itemMin.x + c * (componentWidth + style.ItemInnerSpacing.x) + grabHalf;
				const f32 right = left + componentWidth - grabHalf * 2.f;

				for (i32 t = 0; t <= tickCount; t++)
				{
					const f32 x = left + (right - left) * (static_cast<f32>(t) / tickCount);
					pDraw->AddLine(ImVec2(x, tickTop), ImVec2(x, itemMax.y - 1.f), color, 1.f);
				}
			}
		}
	};
}