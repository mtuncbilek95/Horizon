#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <string>

namespace Horizon::PAL
{
	class H_EXPORT DateTime
	{
	public:
		static constexpr i64 InvalidTicks = i64_min;
		static constexpr i64 TicksPerMillisecond = 10000;
		static constexpr i64 TicksPerSecond = 10000000;
		static constexpr i64 TicksPerMinute = TicksPerSecond * 60;
		static constexpr i64 TicksPerHour = TicksPerMinute * 60;
		static constexpr i64 TicksPerDay = TicksPerHour * 24;

	public:
		DateTime() = default;
		explicit DateTime(i64 ticks) : m_ticks(ticks) {}
		~DateTime() = default;

		static DateTime Now();
		static DateTime FromComponents(i32 year, u32 month, u32 day, u32 hour, u32 minute, u32 second, u32 millisecond);
		static DateTime FromStringToDateTime(const std::string& value);

		std::string ToString() const;

		b8 IsValid() const { return m_ticks != InvalidTicks; }
		i64 GetTicks() const { return m_ticks; }
		i64 GetUnixSeconds() const;

		b8 operator==(const DateTime& other) const { return m_ticks == other.m_ticks; }
		b8 operator!=(const DateTime& other) const { return m_ticks != other.m_ticks; }
		b8 operator<(const DateTime& other) const { return m_ticks < other.m_ticks; }
		b8 operator>(const DateTime& other) const { return m_ticks > other.m_ticks; }
		b8 operator<=(const DateTime& other) const { return m_ticks <= other.m_ticks; }
		b8 operator>=(const DateTime& other) const { return m_ticks >= other.m_ticks; }

	private:
		i64 m_ticks = InvalidTicks;
	};
}