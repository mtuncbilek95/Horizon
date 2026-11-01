#pragma once

#include <Runtime/RHI/Command/ColorAttachment.h>
#include <Runtime/RHI/Command/DepthAttachment.h>

namespace Horizon::RHI
{
	constexpr u32 kMaxColorAttachments = 8;

	struct RenderingDesc final
	{
		ColorAttachment colorAttachments[kMaxColorAttachments] = {};
		u32 colorAttachmentCount = 0;
		DepthAttachment depthAttachment;
	};
}
