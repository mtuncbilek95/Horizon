#pragma once

#include <Runtime/Graphics/RHI/GfxObject.h>

#include <glm/glm.hpp>

#include <span>
#include <array>

namespace Horizon
{
	class GfxBuffer;
	class GfxTexture;
	class GfxPipeline;

	struct GfxTextureBarrier
	{
		GfxTexture* texture = nullptr;
		GfxResourceState before = GfxResourceState::Common;
		GfxResourceState after = GfxResourceState::Common;
	};

	struct GfxBufferBarrier
	{
		GfxBuffer* buffer = nullptr;
		GfxResourceState before = GfxResourceState::Common;
		GfxResourceState after = GfxResourceState::Common;
	};

	struct GfxColorAttachment
	{
		GfxTexture* target = nullptr;
		GfxLoadOp load = GfxLoadOp::Clear;
		GfxStoreOp store = GfxStoreOp::Store;
		glm::vec4 clear = { 0, 0, 0, 1 };
	};

	struct GfxDepthAttachment
	{
		GfxTexture* target = nullptr;
		GfxLoadOp load = GfxLoadOp::Clear;
		GfxStoreOp store = GfxStoreOp::Store;
		f32 clearDepth = 0.0f;
	};

	struct GfxRenderPassBegin
	{
		std::array<GfxColorAttachment, 8> colors;
		u32 colorCount = 0;
		GfxDepthAttachment depth;
		b8 hasDepth = false;
		u32 width = 0, height = 0;
	};

	class GfxCommandList
	{
	public:
		GfxCommandList(GfxDevice* pDevice, GfxQueueType type, GfxHandle allocator);
		~GfxCommandList();

		void Reset(GfxHandle allocator);
		void Close();

		void Barrier(std::span<const GfxTextureBarrier> barriers);
		void Barrier(std::span<const GfxBufferBarrier> barriers);

		void BeginRenderPass(const GfxRenderPassBegin& pass);
		void EndRenderPass();

		void BindPipeline(GfxPipeline* pipeline);
		void SetConstants(const void* data, u32 count);

		void BindIndexBuffer(GfxBuffer* buffer, GfxIndexType type);

		void Draw(u32 vertexCount, u32 instanceCount);
		void DrawIndexed(u32 indexCount, u32 instanceCount, u32 firstIndex = 0, i32 vertexOffset = 0);

		void Dispatch(u32 groupCountX, u32 groupCountY, u32 groupCountZ);
		void DispatchMesh(u32 groupCountX, u32 groupCountY, u32 groupCountZ);

		GfxQueueType GetType() const { return m_type; }
		GfxHandle GetNative() const { return m_native; }

	private:
		GfxDevice* m_device = nullptr;
		GfxHandle m_native = nullptr;
		GfxQueueType m_type = GfxQueueType::Graphics;
	};
}
