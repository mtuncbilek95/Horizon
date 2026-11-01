#pragma once

namespace Horizon::RHI
{
	class CommandPool;

	struct CommandListDesc final
	{
		CommandPool* pPool = nullptr;
	};
}
