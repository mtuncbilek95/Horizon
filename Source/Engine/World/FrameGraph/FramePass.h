#pragma once

namespace Horizon
{
	class FrameGraph;
	class GraphBuilder;
	class PassResources;
	class GfxCommandBuffer;

	class FramePass
	{
		friend class FrameGraph;

	public:
		virtual ~FramePass() = default;

		const FrameGraph& GetGraph() const { return *m_owner; }

		virtual void Setup(GraphBuilder& builder) = 0;
		virtual void Execute(GfxCommandBuffer* cmd, const PassResources& resources) = 0;

	private:
		void SetGraph(FrameGraph* pOwner) { m_owner = pOwner; }

	private:
		FrameGraph* m_owner;
	};
}