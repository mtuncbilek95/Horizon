#pragma once

namespace Horizon::RHI
{
	class CommandList;
}

namespace Horizon::Engine
{
	class PresentationOverlay
	{
	public:
		virtual ~PresentationOverlay() = default;

		virtual void OnRecord(RHI::CommandList* pCmdList) = 0;
		virtual void OnFrameEnd() = 0;
	};
}