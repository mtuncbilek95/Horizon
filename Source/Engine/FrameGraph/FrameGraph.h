#pragma once

#include <Engine/FrameGraph/FrameGraphHandle.h>
#include <Engine/FrameGraph/FrameGraphBuilder.h>
#include <Engine/FrameGraph/FrameGraphResources.h>

#include <Engine/Graphics/RHI/GfxBuffer.h>
#include <Engine/Graphics/RHI/GfxTexture.h>
#include <Engine/Graphics/RHI/GfxCommandList.h>

#include <functional>
#include <memory>
#include <string_view>
#include <vector>

namespace Horizon
{
	static constexpr u32 MaxRenderPass = 64;
	static constexpr u32 MaxTextureResources = 128;
	static constexpr u32 MaxBufferResources = 128;

	struct FrameGraphAccess
	{
		FrameGraphResourceType type = FrameGraphResourceType::Texture;
		u32 resourceIndex = ~0u;
		u32 version = 0;
		GfxResourceState state = GfxResourceState::Common;
	};

	struct FrameGraphColorAttachment
	{
		u32 resourceIndex = ~0u;
		GfxLoadOp  load = GfxLoadOp::Clear;
		GfxStoreOp store = GfxStoreOp::Store;
	};

	struct FrameGraphDepthAttachment
	{
		u32 resourceIndex = ~0u;
		GfxLoadOp  load = GfxLoadOp::Clear;
		GfxStoreOp store = GfxStoreOp::Store;
		b8 enabled = false;
	};

	struct FrameGraphTextureResource
	{
		std::string_view name;
		GfxTextureDesc desc{};
		GfxTextureHandle physical;

		GfxResourceState initialState = GfxResourceState::Common;
		GfxResourceState currentState = GfxResourceState::Common;

		b8 imported = false;
		u32 version = 0;

		u32 refCount = 0;
		i32 firstPass = -1;
		i32 lastPass = -1;

		u32 shaderViewIndex;
		u32 accessViewIndex;

		i32 poolEntry = -1;
	};

	struct FrameGraphBufferResource
	{
		std::string_view name;
		GfxBufferDesc desc{};
		GfxBufferHandle physical;

		GfxResourceState initialState = GfxResourceState::Common;
		GfxResourceState currentState = GfxResourceState::Common;

		b8 imported = false;
		u32 version = 0;

		u32 refCount = 0;
		i32 firstPass = -1;
		i32 lastPass = -1;

		u32 shaderViewIndex;
		u32 accessViewIndex;

		i32 poolEntry = -1;
	};

	struct FrameGraphTexturePoolEntry
	{
		GfxTextureDesc desc{};
		GfxTextureHandle texture;

		GfxResourceState state = GfxResourceState::Common;

		u32 shaderViewIndex = ~0u;
		u32 accessViewIndex = ~0u;
		b8 inUse = false;
	};

	struct FrameGraphBufferPoolEntry
	{
		GfxBufferDesc desc{};
		GfxBufferHandle buffer;

		GfxResourceState state = GfxResourceState::Common;

		u32 shaderViewIndex = ~0u;
		u32 accessViewIndex = ~0u;
		b8 inUse = false;
	};

	class FrameGraphPass
	{
	public:
		virtual ~FrameGraphPass() = default;

		virtual void Execute(FrameGraphResources& resources, GfxCommandList* cmdList) = 0;

		std::string_view m_name;

		std::vector<FrameGraphAccess> m_reads;
		std::vector<FrameGraphAccess> m_writes;
		std::vector<GfxTextureBarrier> m_textureBarriers;
		std::vector<GfxBufferBarrier> m_bufferBarriers;

		std::array<FrameGraphColorAttachment, 8> m_colorAttachments{};
		FrameGraphDepthAttachment m_depthAttachment{};

		FrameGraphQueue m_queue = FrameGraphQueue::Graphics;
		u32 m_colorCount = 0;

		b8 m_hasSideEffect = false;
		b8 m_culled = false;
		b8 m_raster = false;
	};

	template<typename PassData>
	class FrameGraphLambdaPass final : public FrameGraphPass
	{
	public:
		using ExecuteFn = std::function<void(const PassData&, FrameGraphResources&, GfxCommandList*)>;

		void Execute(FrameGraphResources& resources, GfxCommandList* cmdList) override
		{
			m_execute(m_data, resources, cmdList);
		}

		PassData m_data = {};
		ExecuteFn m_execute;
	};

	class FrameGraph
	{
		friend class FrameGraphBuilder;
		friend class FrameGraphResources;
	public:
		FrameGraph();
		~FrameGraph();

		FGTextureHandle ImportTexture(GfxTextureHandle texture, GfxResourceState currentState,
			std::string_view name, u32 width, u32 height, u32 shaderViewIndex = ~0u);
		FGBufferHandle ImportBuffer(GfxBufferHandle buffer, GfxResourceState currentState, std::string_view name, u32 shaderViewIndex);

		template <typename PassData, typename SetupFn, typename ExecuteFn>
		const PassData& AddRasterPass(std::string_view name, SetupFn&& setup, ExecuteFn&& execute);

		template <typename PassData, typename SetupFn, typename ExecuteFn>
		const PassData& AddComputePass(std::string_view name, SetupFn&& setup, ExecuteFn&& execute);

		void Compile();
		void Execute();
		void Reset();

		void Shutdown();

	private:
		void RegisterPass(std::unique_ptr<FrameGraphPass> pass);

		u32 RegisterTransientTexture(const GfxTextureDesc& desc, std::string_view name);
		u32 RegisterTransientBuffer(const GfxBufferDesc& desc, std::string_view name);
		u32 NextTextureVersion(u32 resourceIndex);
		u32 NextBufferVersion(u32 resourceIndex);

		void CullPasses();

		void ComputeLifetimes();
		void AllocateResources();
		u32 AcquireTextureFromPool(const GfxTextureDesc& desc);
		u32 AcquireBufferFromPool(const GfxBufferDesc& desc);

		void BuildBarriers();

		GfxRenderPassBegin BuildRenderPass(const FrameGraphPass& pass) const;

		template <typename PassData, typename SetupFn, typename ExecuteFn>
		const PassData& AddPass(std::string_view name, FrameGraphQueue queue, b8 raster, SetupFn&& setup, ExecuteFn&& execute);

	private:
		std::vector<std::unique_ptr<FrameGraphPass>> m_passes;

		std::vector<FrameGraphTextureResource> m_textureResources;
		std::vector<FrameGraphBufferResource> m_bufferResources;

		std::vector<FrameGraphTexturePoolEntry> m_texturePool;
		std::vector<FrameGraphBufferPoolEntry> m_bufferPool;

		std::vector<GfxBarrier> m_finalTextureBarriers;
		std::vector<GfxBufferBarrier> m_finalBufferBarriers;
	};

	template <typename PassData, typename SetupFn, typename ExecuteFn>
	const PassData& FrameGraph::AddPass(std::string_view name, FrameGraphQueue queue, b8 raster, SetupFn&& setup, ExecuteFn&& execute)
	{
		auto pass = std::make_unique<FrameGraphLambdaPass<PassData>>();
		pass->m_name = name;
		pass->m_queue = queue;
		pass->m_raster = raster;
		pass->m_execute = std::forward<ExecuteFn>(execute);

		FrameGraphBuilder builder(*this, *pass);
		setup(builder, pass->m_data);

		const PassData& dataRef = pass->m_data;
		RegisterPass(std::move(pass));
		return dataRef;
	}

	template <typename PassData, typename SetupFn, typename ExecuteFn>
	const PassData& FrameGraph::AddRasterPass(std::string_view name, SetupFn&& setup, ExecuteFn&& execute)
	{
		return AddPass<PassData>(name, FrameGraphQueue::Graphics, true, std::forward<SetupFn>(setup), std::forward<ExecuteFn>(execute));
	}

	template <typename PassData, typename SetupFn, typename ExecuteFn>
	const PassData& FrameGraph::AddComputePass(std::string_view name, SetupFn&& setup, ExecuteFn&& execute)
	{
		return AddPass<PassData>(name, FrameGraphQueue::Compute, false, std::forward<SetupFn>(setup), std::forward<ExecuteFn>(execute));
	}

	inline FrameGraph& MainGraph()
	{
		static FrameGraph instance;
		return instance;
	}
}