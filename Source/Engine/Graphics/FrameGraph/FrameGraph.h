#pragma once

#include <Engine/Graphics/FrameGraph/FramePass.h>

namespace Horizon
{
	class FrameGraph
	{
	public:
		void AddPass(std::string_view name, std::function<void(GfxCmdList*)>&& exec)
		{
			m_passes.push_back({ name, std::move(exec) });
		}

		void Execute(GfxCmdList* cmd)
		{
			for (auto& pass : m_passes)
				pass.execute(cmd);
		}

		void Reset() { m_passes.clear(); }

	private:
		std::vector<FramePass> m_passes;
	};
}