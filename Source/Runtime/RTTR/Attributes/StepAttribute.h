#pragma once

#include <Runtime/RTTR/Attribute.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Reflect
{
	class StepAttribute final : public Attribute
	{
		HORIZON_ATTRIBUTE_REFLECT(StepAttribute);
	public:
		StepAttribute() = default;
		StepAttribute(f32 step, f32 fastStep = 0.f) : m_step(step), m_fastStep(fastStep > 0.f ? fastStep : step * 10.f)
		{
		}
		~StepAttribute() = default;

		b8 IsAuto() const { return m_step <= 0.f; }
		f32 GetStep() const { return IsAuto() ? 1.f : m_step; }
		f32 GetFastStep() const { return IsAuto() ? 10.f : m_fastStep; }

	private:
		f32 m_step = 0.f;
		f32 m_fastStep = 0.f;
	};
}