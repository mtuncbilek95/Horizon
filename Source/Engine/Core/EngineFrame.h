#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::Engine
{
	class H_EXPORT EngineFrame final
	{
		friend class Engine;
	public:
		f32 DeltaTime() const { return m_delta; }
		f64 GetTotalSeconds() const { return m_total; }
		u64 GetFrameIndex() const { return m_frameIndex; }

	private:
		void Advance(f32 rawDelta)
		{
			if (rawDelta > MaxDelta)
				rawDelta = MaxDelta;

			m_delta = (f32)rawDelta;
			m_total += rawDelta;
			m_frameIndex++;
		}

	private:
		static constexpr f64 MaxDelta = 0.25;

		f64 m_total = 0.0;
		f32 m_delta = 0.0f;

		usize m_frameIndex = 0;
	};
}