#include "ImportContext.h"

namespace Horizon
{
	b8 ImportContext::IsEnabled(std::string_view key) const
	{
		return false;
	}

	i32 ImportContext::GetInt(std::string_view key, i32 fallback) const
	{
		return fallback;
	}
}