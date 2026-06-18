#include "FrameGraph.h"

#include <Runtime/Graphics/RHI/Queue/GfxQueue.h>
#include <Runtime/Graphics/RHI/Command/GfxCommandList.h>

#include <Engine/Job/JobModule.h>
#include <Engine/Graphics/FrameContext.h>
#include <Engine/Graphics/FrameGraph/FrameGraphBuilder.h>
#include <Engine/Graphics/FrameGraph/TransientResourcePool.h>

#include <algorithm>

namespace Horizon
{
	FGTextureHandle FrameGraph::ImportTexture(const char* name, GfxTexture* texture, GfxResourceState initState, GfxResourceState finalState)
	{
		ResourceNode node;
		node.name = name;
		node.imported = texture;
		node.initialState = initState;
		node.finalState = finalState;

		m_resources.push_back(std::move(node));
		return FGTextureHandle{ u32(m_resources.size() - 1) };
	}

	FGTextureHandle FrameGraph::CreateTexture(const char* name, const GfxTextureDesc& desc)
	{
		ResourceNode node;
		node.name = name;
		node.desc = desc;
		m_resources.push_back(std::move(node));

		return FGTextureHandle{ u32(m_resources.size() - 1) };
	}

	void FrameGraph::AddPass(const char* name, FrameGraphSetupFn setup, FrameGraphExecuteFn execute)
	{
		const u32 passIndex = u32(m_passes.size());
		PassNode pass;
		pass.name = name;
		pass.execute = std::move(execute);
		m_passes.push_back(std::move(pass));

		FrameGraphBuilder builder(*this, passIndex);
		setup(builder);
	}

	void FrameGraph::MarkSideEffect()
	{
		if (!m_passes.empty())
			m_passes.back().hasSideEffect = true;
	}

	void FrameGraph::RegisterRead(u32 passIndex, FGTextureHandle handle, GfxResourceState state)
	{
		m_passes[passIndex].reads.push_back({ handle.index, state });
	}

	void FrameGraph::RegisterWrite(u32 passIndex, FGTextureHandle handle, GfxResourceState state)
	{
		m_passes[passIndex].writes.push_back({ handle.index, state });
		m_resources[handle.index].producer = i32(passIndex);
	}

	void FrameGraph::ResolveResources(TransientResourcePool& pool)
	{
		m_physicalTextures.assign(m_resources.size(), nullptr);

		for (u32 r = 0; r < m_resources.size(); ++r)
		{
			if (m_resources[r].IsImported())
				m_physicalTextures[r] = m_resources[r].imported;
		}

		for (i32 pos = 0; pos < i32(m_order.size()); ++pos)
		{
			for (u32 r = 0; r < m_resources.size(); ++r)
			{
				ResourceNode& res = m_resources[r];

				if (!res.IsImported() && res.firstUse == pos)
					m_physicalTextures[r] = pool.AcquireTexture(res.desc);
			}

			for (u32 r = 0; r < m_resources.size(); ++r)
			{
				ResourceNode& res = m_resources[r];

				if (!res.IsImported() && res.lastUse == pos)
					pool.ReleaseTexture(m_physicalTextures[r]);
			}
		}
	}

	void FrameGraph::RecordPass(u32 orderPos, FrameContext& frame)
	{
		PassNode& pass = m_passes[m_order[orderPos]];

		GfxCommandList* cmd = frame.AcquireCommandList(pass.queue);
		cmd->SetupBindless();

		IssueBarriers(cmd, pass.entryBarriers);

		FrameGraphResources resources(m_physicalTextures);
		pass.execute(cmd, resources);

		cmd->End();
		m_recordedLists[orderPos] = cmd;
	}

	void FrameGraph::IssueBarriers(GfxCommandList* cmd, const std::vector<FGBarrier>& barriers)
	{
		if (barriers.empty())
			return;

		std::vector<GfxTextureBarrier> native;
		native.reserve(barriers.size());

		for (const FgBarrier& b : barriers)
			native.push_back({ m_physicalTextures[b.resource], b.before, b.after });

		for (usize off = 0; off < native.size(); off += 16)
		{
			const u32 count = u32(std::min<usize>(16, native.size() - off));
			cmd->Barrier(native.data() + off, count);
		}
	}

	void FrameGraph::Cull()
	{
		for (ResourceNode& res : m_resources)
			res.refCount = res.IsImported() ? 1u : 0u;

		for (PassNode& pass : m_passes)
		{
			pass.refCount = u32(pass.writes.size());
			for (const ResourceAccess& r : pass.reads)
				++m_resources[r.resource].refCount;
		}

		std::vector<u32> stack;
		for (u32 r = 0; r < m_resources.size(); ++r)
		{
			if (m_resources[r].refCount == 0)
				stack.push_back(r);
		}

		while (!stack.empty())
		{
			const u32 r = stack.back();
			stack.pop_back();

			const i32 p = m_resources[r].producer;
			if (p < 0)
				continue;

			PassNode& producer = m_passes[p];
			if (producer.hasSideEffect || producer.refCount == 0)
				continue;

			if (--producer.refCount == 0)
			{
				producer.culled = true;
				for (const ResourceAccess& rd : producer.reads)
				{
					ResourceNode& res = m_resources[rd.resource];
					if (res.refCount > 0 && --res.refCount == 0)
						stack.push_back(rd.resource);
				}
			}
		}
	}

	void FrameGraph::BuildOrder()
	{
		m_order.clear();
		for (u32 p = 0; p < m_passes.size(); ++p)
		{
			if (!m_passes[p].culled)
				m_order.push_back(p);
		}
	}

	void FrameGraph::ComputeLifetimes()
	{
		for (ResourceNode& res : m_resources)
		{
			res.firstUse = -1;
			res.lastUse = -1;
		}

		for (i32 pos = 0; pos < i32(m_order.size()); ++pos)
		{
			const PassNode& pass = m_passes[m_order[pos]];
			auto touch = [&](u32 resource)
				{
					ResourceNode& res = m_resources[resource];
					if (res.firstUse < 0)
						res.firstUse = pos;
					res.lastUse = pos;
				};

			for (const ResourceAccess& a : pass.reads)
				touch(a.resource);

			for (const ResourceAccess& a : pass.writes)
				touch(a.resource);
		}
	}

	void FrameGraph::ComputeBarriers()
	{
		for (u32 r = 0; r < m_resources.size(); ++r)
		{
			if (m_resources[r].IsImported())
				m_physicalTextures[r]->SetState(m_resources[r].initialState);
		}

		auto transition = [&](PassNode& pass, const ResourceAccess& a)
			{
				GfxTexture* texture = m_physicalTextures[a.resource];
				const GfxResourceState before = texture->GetState();
				if (before != a.state)
				{
					pass.entryBarriers.push_back({ a.resource, before, a.state });
					texture->SetState(a.state);
				}
			};

		for (u32 passIdx : m_order)
		{
			PassNode& pass = m_passes[passIdx];
			pass.entryBarriers.clear();

			for (const ResourceAccess& a : pass.writes)
				transition(pass, a);

			for (const ResourceAccess& a : pass.reads)
				transition(pass, a);
		}

		m_exitBarriers.clear();
		for (u32 r = 0; r < m_resources.size(); ++r)
		{
			ResourceNode& res = m_resources[r];
			if (!res.IsImported())
				continue;
			GfxTexture* texture = m_physicalTextures[r];
			if (texture->GetState() != res.finalState)
			{
				m_exitBarriers.push_back({ r, texture->GetState(), res.finalState });
				texture->SetState(res.finalState);
			}
		}
	}

	void FrameGraph::Compile(TransientResourcePool& pool)
	{
		Cull();
		BuildOrder();
		ComputeLifetimes();
		ResolveResources(pool);
		ComputeBarriers();
	}

	void FrameGraph::Execute(JobModule& jobs, FrameContext& frame, GfxQueue& queue)
	{
		const usize passCount = m_order.size();
		m_recordedLists.assign(passCount, nullptr);

		JobCounter counter;
		for (usize i = 0; i < passCount; ++i)
			jobs.Dispatch(counter, [this, i, &frame]() { RecordPass(u32(i), frame); });

		jobs.Wait(counter);

		GfxCommandList* finalCmd = nullptr;
		if (!m_exitBarriers.empty())
		{
			finalCmd = frame.AcquireCommandList(GfxQueueType::Graphics);
			IssueBarriers(finalCmd, m_exitBarriers);
			finalCmd->End();
		}

		std::vector<GfxCommandList*> submit;
		submit.reserve(passCount + 1);
		for (GfxCommandList* cmd : m_recordedLists)
		{
			if (cmd)
				submit.push_back(cmd);
		}

		if (finalCmd)
			submit.push_back(finalCmd);

		if (!submit.empty())
			queue.Submit(submit.data(), u32(submit.size()));
	}

	void FrameGraph::Clear()
	{
		m_passes.clear();
		m_resources.clear();
		m_order.clear();
		m_exitBarriers.clear();
		m_physicalTextures.clear();
		m_recordedLists.clear();
	}
}