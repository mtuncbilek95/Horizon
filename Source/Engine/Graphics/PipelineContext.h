#pragma once

#include <Runtime/Graphics/RHI/Pipeline/GfxPipeline.h>

#include <Engine/Core/Context.h>

namespace Horizon
{
	struct PipelineInfo
	{
		std::string cacheName;
		std::string shaderProgram;
		PrimitiveTopology topology = PrimitiveTopology::TriangleList;
		RasterizerState rasterizer;
		DepthStencilState depthStencil;
		BlendState blend;
		DynamicRendering dynamicRendering;
		std::vector<DynamicState> dynamicStates;
		PipelineFlags flags;

		PipelineInfo& setCacheName(const std::string& name) { cacheName = name; return *this; }
		PipelineInfo& setShaderProgram(const std::string& name) { shaderProgram = name; return *this; }
		PipelineInfo& setTopology(PrimitiveTopology top) { topology = top; return *this; }
		PipelineInfo& setRasterizer(const RasterizerState& rs) { rasterizer = rs; return *this; }
		PipelineInfo& setDepthStencil(const DepthStencilState& ds) { depthStencil = ds; return *this; }
		PipelineInfo& setBlend(const BlendState& bs) { blend = bs; return *this; }
		PipelineInfo& setDynamicRendering(const DynamicRendering& dr) { dynamicRendering = dr; return *this; }
		PipelineInfo& addDynamicState(DynamicState state) { dynamicStates.push_back(state); return *this; }
		PipelineInfo& setFlags(PipelineFlags f) { flags = f; return *this; }

		bool operator==(const PipelineInfo& other) const
		{
			if (shaderProgram != other.shaderProgram) return false;
			if (topology != other.topology) return false;
			if (std::memcmp(&rasterizer, &other.rasterizer, sizeof(RasterizerState)) != 0) return false;
			if (std::memcmp(&depthStencil, &other.depthStencil, sizeof(DepthStencilState)) != 0) return false;
			if (flags != other.flags) return false;
			if (dynamicStates != other.dynamicStates) return false;

			if (blend.logicEnable != other.blend.logicEnable) return false;
			if (blend.logicOp != other.blend.logicOp) return false;
			if (blend.attachments.size() != other.blend.attachments.size()) return false;
			for (size_t i = 0; i < blend.attachments.size(); i++)
			{
				if (std::memcmp(&blend.attachments[i], &other.blend.attachments[i], sizeof(BlendAttachment)) != 0)
					return false;
			}

			if (dynamicRendering.colorAttachmentFormats != other.dynamicRendering.colorAttachmentFormats) return false;
			if (dynamicRendering.depthAttachmentFormat != other.dynamicRendering.depthAttachmentFormat) return false;
			if (dynamicRendering.stencilAttachmentFormat != other.dynamicRendering.stencilAttachmentFormat) return false;

			return true;
		}
	};
}

namespace std
{
	template<>
	struct std::hash<Horizon::PipelineInfo>
	{
		size_t operator()(const Horizon::PipelineInfo& info) const noexcept
		{
			size_t seed = std::hash<std::string>{}(info.shaderProgram);

			auto combine = [&seed](const void* data, size_t size)
				{
					const auto* bytes = static_cast<const u8*>(data);
					for (size_t i = 0; i < size; i++)
						seed ^= std::hash<u8>{}(bytes[i]) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
				};

			combine(&info.topology, sizeof(info.topology));
			combine(&info.rasterizer, sizeof(info.rasterizer));
			combine(&info.depthStencil, sizeof(info.depthStencil));
			combine(&info.flags, sizeof(info.flags));

			combine(&info.blend.logicEnable, sizeof(info.blend.logicEnable));
			combine(&info.blend.logicOp, sizeof(info.blend.logicOp));
			for (auto& att : info.blend.attachments)
				combine(&att, sizeof(att));

			for (auto& fmt : info.dynamicRendering.colorAttachmentFormats)
				combine(&fmt, sizeof(fmt));
			combine(&info.dynamicRendering.depthAttachmentFormat, sizeof(info.dynamicRendering.depthAttachmentFormat));
			combine(&info.dynamicRendering.stencilAttachmentFormat, sizeof(info.dynamicRendering.stencilAttachmentFormat));

			for (auto& ds : info.dynamicStates)
				combine(&ds, sizeof(ds));

			return seed;
		}
	};
}

namespace Horizon
{
	class PipelineContext : public Context<PipelineContext>
	{
	public:
		GfxPipeline* GetOrCreate(const PipelineInfo& desc);

	private:
		EngineReport OnInitialize() final;
		void OnFinalize() final;

	private:
		std::unordered_map<PipelineInfo, std::shared_ptr<GfxPipeline>> m_pipelines;
	};
}