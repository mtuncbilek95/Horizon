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

	b8 StringOps::EqualsNoCase(std::string_view left, std::string_view right)
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

	std::string StringOps::NoExtension(const std::string& path)
	{
		usize index = path.rfind('.');
		return path.substr(0, index);
	}
	
	std::string StringOps::OnlyExtension(const std::string& path)
	{
		usize index = path.rfind('.');
		if (index == std::string::npos)
			return "";

		return path.substr(index);
	}

	std::string StringOps::ToRelative(const std::string& absolutePath, const std::string& trimPath, const std::string& rootOf)
	{
		if (!StringOps::StartsWithNoCase(absolutePath, trimPath))
			return std::string();

		usize cursor = trimPath.size();

		while (cursor < absolutePath.size() && StringOps::IsSeparator(absolutePath[cursor]))
			++cursor;

		return rootOf + "/" + absolutePath.substr(cursor);
	}

	std::string StringOps::ToAbsolute(const std::string& relativePath, const std::string& missingPath, const std::string& rootOf)
	{
		if (!relativePath.starts_with(rootOf))
			return std::string();

		return missingPath + relativePath.substr(rootOf.size());
	}

	std::string StringOps::RootOf(const std::string& relativePath)
	{
		if (relativePath.empty() || relativePath[0] != '[')
			return std::string();

		usize close = relativePath.find(']');

		if (close == std::string::npos || close + 1 >= relativePath.size() || relativePath[close + 1] != ':')
			return std::string();

		return relativePath.substr(0, close + 2);
	}

	std::string_view StringOps::GetNameString(std::string_view name)
	{
		constexpr std::string_view prefixes[] = { "class ", "struct ", "enum ", "union " };

		for (std::string_view prefix : prefixes)
		{
			if (name.starts_with(prefix))
			{
				name.remove_prefix(prefix.size());
				break;
			}
		}

		return name;
	}

	std::string_view StringOps::ParseName(std::string_view name)
	{
		if (usize pos = name.rfind("::"); pos != std::string_view::npos)
			return name.substr(pos + 2);

		if (usize pos = name.find(' '); pos != std::string_view::npos)
			return name.substr(pos + 1);

		return name;
	}
}