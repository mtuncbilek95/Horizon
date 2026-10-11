#include "SearchFilter.h"

namespace Horizon::Editor
{
	b8 SearchFilter::Matches(const std::string& name, const std::string& search)
	{
		if (search.empty())
			return true;

		if (search.size() > name.size())
			return false;

		for (usize start = 0; start + search.size() <= name.size(); start++)
		{
			usize i = 0;

			while (i < search.size() && std::tolower(static_cast<u8>(name[start + i])) == std::tolower(static_cast<u8>(search[i])))
				i++;

			if (i == search.size())
				return true;
		}

		return false;
	}

}