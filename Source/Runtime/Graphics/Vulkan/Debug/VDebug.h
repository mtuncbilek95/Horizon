#pragma once

#include <vulkan/vulkan.h>

#include <string>

namespace Horizon
{
	struct VDebug final
	{
		static void VkAssert(VkResult result, const std::string& objectTitle);
	};
}
