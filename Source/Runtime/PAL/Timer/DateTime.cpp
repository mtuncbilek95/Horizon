#include <Runtime/PAL/Timer/DateTime.h>

#include <Runtime/Log/Terminal.h>

#include <cstdio>

namespace Horizon::PAL
{
	namespace
	{
		i64 FloorDiv(i64 value, i64 divisor)
		{
			i64 quotient = value / divisor;

			if (value % divisor != 0 && (value < 0) != (divisor < 0))
				--quotient;

			return quotient;
		}

		i64 FloorMod(i64 value, i64 divisor)
		{
			return value - FloorDiv(value, divisor) * divisor;
		}

		i64 DaysFromCivil(i64 year, i64 month, i64 day)
		{
			year -= month <= 2;

			i64 era = FloorDiv(year, 400);
			i64 yoe = year - era * 400;
			i64 doy = (153 * (month + (month > 2 ? -3 : 9)) + 2) / 5 + day - 1;
			i64 doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;

			return era * 146097 + doe - 719468;
		}

		void CivilFromDays(i64 days, i64& outYear, i64& outMonth, i64& outDay)
		{
			days += 719468;

			i64 era = FloorDiv(days, 146097);
			i64 doe = days - era * 146097;
			i64 yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
			i64 doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
			i64 mp = (5 * doy + 2) / 153;

			outDay = doy - (153 * mp + 2) / 5 + 1;
			outMonth = mp + (mp < 10 ? 3 : -9);
			outYear = yoe + era * 400 + (outMonth <= 2);
		}

		i64 DaysInMonth(i64 year, i64 month)
		{
			if (month == 2)
			{
				b8 isLeap = (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
				return isLeap ? 29 : 28;
			}

			if (month == 4 || month == 6 || month == 9 || month == 11)
				return 30;

			return 31;
		}

		b8 ReadDigits(const std::string& value, usize& cursor, usize count, i64& outValue)
		{
			if (cursor + count > value.size())
				return false;

			i64 result = 0;

			for (usize i = 0; i < count; ++i)
			{
				c8 digit = value[cursor + i];

				if (digit < '0' || digit > '9')
					return false;

				result = result * 10 + (digit - '0');
			}

			cursor += count;
			outValue = result;

			return true;
		}

		b8 Expect(const std::string& value, usize& cursor, c8 expected)
		{
			if (cursor >= value.size() || value[cursor] != expected)
				return false;

			++cursor;
			return true;
		}
	}

	DateTime DateTime::FromComponents(i32 year, u32 month, u32 day, u32 hour, u32 minute, u32 second, u32 millisecond)
	{
		b8 inRange = month >= 1 && month <= 12 && day >= 1 && day <= (u32)DaysInMonth(year, month) &&
			hour <= 23 && minute <= 59 && second <= 59 && millisecond <= 999;

		if (!inRange)
		{
			Terminal::Error("DateTime", "{}-{}-{} {}:{}:{}.{} is not a valid civil time",
				year, month, day, hour, minute, second, millisecond);
			return DateTime();
		}

		i64 ticks = DaysFromCivil(year, month, day) * TicksPerDay + hour * TicksPerHour +
			minute * TicksPerMinute + second * TicksPerSecond + millisecond * TicksPerMillisecond;

		return DateTime(ticks);
	}

	DateTime DateTime::FromStringToDateTime(const std::string& value)
	{
		usize cursor = 0;

		i64 year = 0;
		i64 month = 0;
		i64 day = 0;
		i64 hour = 0;
		i64 minute = 0;
		i64 second = 0;
		i64 millisecond = 0;

		b8 parsed = ReadDigits(value, cursor, 4, year) && Expect(value, cursor, '-') &&
			ReadDigits(value, cursor, 2, month) && Expect(value, cursor, '-') &&
			ReadDigits(value, cursor, 2, day);

		if (!parsed)
		{
			Terminal::Error("DateTime", "{} does not begin with an ISO-8601 date", value);
			return DateTime();
		}

		if (cursor < value.size() && (value[cursor] == 'T' || value[cursor] == ' '))
		{
			++cursor;

			parsed = ReadDigits(value, cursor, 2, hour) && Expect(value, cursor, ':') &&
				ReadDigits(value, cursor, 2, minute) && Expect(value, cursor, ':') &&
				ReadDigits(value, cursor, 2, second);

			if (!parsed)
			{
				Terminal::Error("DateTime", "{} has a malformed time of day", value);
				return DateTime();
			}

			if (cursor < value.size() && value[cursor] == '.')
			{
				++cursor;

				usize digits = 0;

				while (cursor < value.size() && value[cursor] >= '0' && value[cursor] <= '9')
				{
					if (digits < 3)
						millisecond = millisecond * 10 + (value[cursor] - '0');

					++digits;
					++cursor;
				}

				if (digits == 0)
				{
					Terminal::Error("DateTime", "{} has an empty fractional second", value);
					return DateTime();
				}

				while (digits < 3)
				{
					millisecond *= 10;
					++digits;
				}
			}
		}

		if (cursor < value.size() && value[cursor] == 'Z')
			++cursor;

		if (cursor != value.size())
		{
			Terminal::Error("DateTime", "{} has trailing characters after the time point", value);
			return DateTime();
		}

		return FromComponents((i32)year, (u32)month, (u32)day, (u32)hour, (u32)minute, (u32)second, (u32)millisecond);
	}

	std::string DateTime::ToString() const
	{
		if (!IsValid())
		{
			Terminal::Error("DateTime", "An invalid time point cannot be stringified");
			return std::string();
		}

		i64 dayCount = FloorDiv(m_ticks, TicksPerDay);
		i64 timeOfDay = FloorMod(m_ticks, TicksPerDay);

		i64 year = 0;
		i64 month = 0;
		i64 day = 0;

		CivilFromDays(dayCount, year, month, day);

		c8 buffer[40] = {};
		i32 length = std::snprintf(buffer, sizeof(buffer), "%04lld-%02lld-%02lldT%02lld:%02lld:%02lld.%03lldZ",
			year, month, day, timeOfDay / TicksPerHour, timeOfDay / TicksPerMinute % 60,
			timeOfDay / TicksPerSecond % 60, timeOfDay / TicksPerMillisecond % 1000);

		if (length <= 0)
		{
			Terminal::Error("DateTime", "{} ticks could not be formatted", m_ticks);
			return std::string();
		}

		return std::string(buffer, (usize)length);
	}

	i64 DateTime::GetUnixSeconds() const
	{
		if (!IsValid())
			return 0;

		return FloorDiv(m_ticks, TicksPerSecond);
	}
}