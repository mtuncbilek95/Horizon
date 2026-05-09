#pragma once

#include <Runtime/Graphics/RHI/Pipeline/GfxPipelineDesc.h>

namespace Horizon
{
	struct BlendMode
	{
		enum class Values
		{
			Opaque,
			AlphaBlend,
			Additive,
			Premultiplied
		};

		static BlendAttachment blendAttachmentFromMode(BlendMode::Values mode);
	};
}