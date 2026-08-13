#include "StringOps.h"

namespace Horizon
{
	b8 StringOps::IsSeparator(c8 value)
	{
		return value == '\\' || value == '/';
	}

	c8 StringOps::ToLowerAscii(c8 value)
	{
		if (value >= 'A' && value <= 'Z')
			return (char)(value - 'A' + 'a');

		return value;
	}

	b8 StringOps::StartsWithNoCase(const std::string& value, const std::string& prefix)
	{
		if (value.size() < prefix.size())
			return false;

		for (usize i = 0; i < prefix.size(); ++i)
		{
			char left = ToLowerAscii(value[i]);
			char right = ToLowerAscii(prefix[i]);

			if (IsSeparator(left) && IsSeparator(right))
				continue;

			if (left != right)
				return false;
		}

		return true;
	}

	b8 StringOps::EqualsNoCase(const std::string& left, const std::string& right)
	{
		if (left.size() != right.size())
			return false;

		for (usize i = 0; i < left.size(); ++i)
		{
			if (ToLowerAscii(left[i]) != ToLowerAscii(right[i]))
				return false;
		}

		return true;
	}

	std::string StringOps::ParentPathOf(const std::string& path)
	{
		usize end = path.size();

		while (end > 0 && IsSeparator(path[end - 1]))
			--end;

		while (end > 0 && !IsSeparator(path[end - 1]))
			--end;

		while (end > 0 && IsSeparator(path[end - 1]))
			--end;

		return path.substr(0, end);
	}

}