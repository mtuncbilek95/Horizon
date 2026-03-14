#pragma once

namespace Horizon
{
	enum class CommandPoolFlags
	{
		ResetCommandBuffer = 1 << 0
	};

	enum class CommandLevel
	{
		Primary,
		Secondary
	};

	enum class CommandBufferUsage
	{
		OneTimeSubmit = 1 << 0,
		RenderPassContinue = 1 << 1,
		SimultaneousUse = 1 << 2
	};

	enum class SubpassContent
	{
		ContentsInline,
		SecondaryBuffer,
		InlineAndSecondaryBuffer
	};
}
