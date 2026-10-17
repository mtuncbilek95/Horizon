#include "StringOps.h"

#include <Runtime/Containers/List.h>

namespace Horizon
{
	void StringOps::CopyToChar(c8* pDst, const std::string& src)
	{
		std::memcpy(pDst, src.data(), src.size());
	}

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

	std::string StringOps::NormalizePath(const std::string& path)
	{
		std::string res;
		res.reserve(path.size());

		for (c8 value : path)
		{
			if (!IsSeparator(value))
				res.push_back(value);
			else if (res.empty() || res.back() != '/')
				res.push_back('/');
		}

		if (res.size() > 1 && res.back() == '/')
			res.pop_back();

		return res;
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

	std::string StringOps::ResolveRelativePath(const std::string& relativeParent, const std::string& path)
	{
		auto collapse = [](const std::string& raw)
			{
				const std::string normalized = NormalizePath(raw);
				const b8 isRooted = !normalized.empty() && normalized[0] == '/';

				List<std::string> segments;
				usize offset = 0;

				while (offset <= normalized.size())
				{
					usize separator = normalized.find('/', offset);
					if (separator == std::string::npos)
						separator = normalized.size();

					const std::string segment = normalized.substr(offset, separator - offset);
					offset = separator + 1;

					if (segment.empty() || segment == ".")
						continue;

					if (segment == "..")
					{
						if (segments.GetCount() > 0)
							segments.RemoveAt(segments.GetCount() - 1);

						continue;
					}

					segments.PushBack(segment);
				}

				std::string result = isRooted ? "/" : "";
				for (usize i = 0; i < segments.GetCount(); ++i)
				{
					if (i > 0)
						result += "/";

					result += segments[i];
				}

				return result;
			};

		const std::string normalized = NormalizePath(path);

		const b8 isAbsolute = (!normalized.empty() && normalized[0] == '/') ||
			(normalized.size() > 1 && normalized[1] == ':');

		if (isAbsolute || relativeParent.empty())
			return collapse(normalized);

		return collapse(relativeParent + "/" + normalized);
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