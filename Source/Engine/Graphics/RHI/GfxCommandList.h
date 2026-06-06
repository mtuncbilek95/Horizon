#pragma once

#include <Engine/Graphics/RHI/GfxDevice.h>

#include <glm/glm.hpp>

#include <span>
#include <array>
#include <string_view>

namespace Horizon
{
	struct GfxBarrier
	{
		GfxTextureHandle texture;
		GfxResourceState before, after;
	};

	struct GfxBufferBarrier
	{
		GfxBufferHandle buffer;
		GfxResourceState before, after;
	};

	struct GfxColorAttachment
	{
		GfxTextureHandle target;
		GfxLoadOp load = GfxLoadOp::Clear;
		GfxStoreOp store = GfxStoreOp::Store;
		glm::vec4 clear = { 0, 0, 0, 1 };
	};

	struct GfxDepthAttachment
	{
		GfxTextureHandle target;
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

	namespace GfxCmdList
	{
		void Barrier(GfxCommandList* cmdList, std::span<const GfxBarrier> barriers);
		void Barrier(GfxCommandList* cmdList, std::span<const GfxBufferBarrier> barriers);

		void BeginMarker(GfxCommandList* cmdList, std::string_view name, const glm::uvec3& color);
		void EndMarker(GfxCommandList* cmdList);
		void Marker(GfxCommandList* cmdList, std::string_view name);

		void BeginRenderPass(GfxCommandList* cmdList, const GfxRenderPassBegin& renderPass);
		void EndRenderPass(GfxCommandList* cmdList);

		void BindPipeline(GfxCommandList* cmdList, GfxPipelineHandle pipeline);
		void SetConstants(GfxCommandList* cmdList, const void* data, u32 size);

		void BindIndexBuffer(GfxCommandList* cmdList, GfxBufferHandle buffer, GfxIndexType type);

		void Draw(GfxCommandList* cmdList, u32 vertexCount, u32 instanceCount);
		void DrawIndexed(GfxCommandList* cmdList, u32 indexCount, u32 instanceCount, u32 firstIndex = 0, i32 vertexOffset = 0);
		void ExecuteIndirect(GfxCommandList* cmdList, GfxCommandSignatureHandle signature, u32 maxCommandCount, GfxBufferHandle argBuffer, u64 argOffset = 0,
			GfxBufferHandle countBuffer = {}, u64 countOffset = 0);

		void Dispatch(GfxCommandList* cmdList, u32 groupCountX, u32 groupCountY, u32 groupCountZ);
		void DispatchMesh(GfxCommandList* cmdList, u32 groupCountX, u32 groupCountY, u32 groupCountZ);
	}
}