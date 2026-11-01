#include "StringHelper.h"

namespace Horizon
{
	b8 StringHelper::IsCharSeparator(c8 value)
	{
		return value == '\\' || value == '/';
	}

	b8 StringHelper::CheckIfEquals(std::string_view left, std::string_view right)
	{
		if (left.size() != right.size())
			return false;

		for (usize i = 0; i < left.size(); ++i)
		{
			if (std::tolower(left[i]) != std::tolower(right[i]))
				return false;
		}

		return true;
	}

	b8 StringHelper::CheckIfEqualsCaseSensitive(std::string_view left, std::string_view right)
	{
		return true;
	}

	std::string StringHelper::NormalizePath(const std::string& path)
	{
		std::string res;
		res.reserve(path.size());

		for (c8 value : path)
		{
			if (!IsCharSeparator(value))
				res.push_back(value);
			else if (res.empty() || res.back() != '/')
				res.push_back('/');
		}

		if (res.size() > 1 && res.back() == '/')
			res.pop_back();

		return res;
	}

	std::string StringHelper::FindParentPath(const std::string& path)
	{
		usize end = path.size();

		while (end > 0 && IsCharSeparator(path[end - 1]))
			--end;

		while (end > 0 && !IsCharSeparator(path[end - 1]))
			--end;

		while (end > 0 && IsCharSeparator(path[end - 1]))
			--end;

		return path.substr(0, end);
	}

	std::string StringHelper::ResolveWithNoExtension(const std::string path)
	{
		usize index = path.rfind('.');
		return path.substr(0, index);
	}

	std::string StringHelper::FindExtension(const std::string path)
	{
		usize index = path.rfind('.');
		if (index == std::string::npos)
			return "";

		return path.substr(index);
	}

	std::string StringHelper::FindFileName(const std::string& path)
	{
		usize end = path.size();

		while (end > 0 && IsCharSeparator(path[end - 1]))
			--end;

		usize start = end;

		while (start > 0 && !IsCharSeparator(path[start - 1]))
			--start;

		return path.substr(start, end - start);
	}

	std::string_view StringHelper::GetNameString(std::string_view name)
	{
#if defined(HORIZON_WINDOWS)
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
#else
#error "Not Implemented Yet"
#endif
	}

	std::string_view StringHelper::ParseTypeName(std::string_view typeName)
	{
#if defined(HORIZON_WINDOWS)
		if (usize pos = typeName.rfind("::"); pos != std::string_view::npos)
			return typeName.substr(pos + 2);

		if (usize pos = typeName.find(' '); pos != std::string_view::npos)
			return typeName.substr(pos + 1);

		return typeName;
#else
#error "Not Implemented Yet"
#endif
	}
}