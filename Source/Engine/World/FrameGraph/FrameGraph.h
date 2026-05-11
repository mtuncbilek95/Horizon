#pragma once

#include <Engine/World/FrameGraph/ResourceHandle.h>
#include <Engine/World/FrameGraph/RenderTargetDesc.h>
#include <Engine/World/FrameGraph/RenderTargetEntry.h>
#include <Engine/World/FrameGraph/GraphBuilder.h>
#include <Engine/World/FrameGraph/PassResources.h>
#include <Engine/World/FrameGraph/FrameGraphCache.h>
#include <Engine/World/FrameGraph/FramePass.h>
#include <Engine/World/Renderer/RenderableObject.h>
#include <Engine/World/Renderer/ViewObject.h>

#include <Engine/Presentation/PresentationObjects.h>

#include <functional>
#include <string>

namespace Horizon
{
	class GfxCommandBuffer;
	class PassResources;

	struct ResourceEntry
	{
		RenderTargetDesc desc;
		u32 writer = u32_max;
		std::vector<u32> readers;
		RenderTargetEntry* resource = nullptr;
		GfxImageView* importedView = nullptr;
		b8 imported = false;
	};

	class FrameGraph
	{
		friend class GraphBuilder;

	public:
		void SetGraphCache(std::unique_ptr<FrameGraphCache> cache);
		void Import(const CompositePresentObject& presentObj);

		template<typename T>
			requires std::derived_from<T, FramePass>
		void RegisterPass(const std::string& passName)
		{
			auto it = m_passAuxiliary.find(passName);
			if (it != m_passAuxiliary.end())
			{
				Log::Terminal(LogType::Error, "FrameGraph", "{} has been already registered!!!!", passName);
				return;
			}

			auto pass = std::make_unique<T>();
			pass->SetGraph(this);
			m_passAuxiliary[passName] = static_cast<u32>(m_passes.size());
			m_passes.push_back(std::move(pass));
			m_passNames.push_back(passName);
		}

		void Compile();
		void Execute(GfxCommandBuffer* cmd);
		void Reset();

		void SetRenderables(std::vector<RenderableObject>&& renderables) { m_renderables = std::move(renderables); }
		void SetViewObject(ViewObject&& obj) { m_viewObject = std::move(obj); }

		const std::vector<RenderableObject>& GetRenderables() const { return m_renderables; }
		const ViewObject& GetViewObject() const { return m_viewObject; }

	private:
		RenderTargetHandle CreateResource(const std::string& name, const RenderTargetDesc& desc);
		void RegisterRead(u32 passIndex, RenderTargetHandle handle);
		void RegisterWrite(u32 passIndex, RenderTargetHandle handle);

	private:
		std::vector<ResourceEntry> m_resources;
		std::vector<u32> m_executionOrder;

		std::unique_ptr<FrameGraphCache> m_cache;

		std::vector<std::string> m_passNames;
		std::vector<std::unique_ptr<FramePass>> m_passes;
		std::unordered_map<std::string, u32> m_passAuxiliary;
		std::unordered_map<std::string, RenderTargetHandle> m_handleRegistry;
		RenderTargetHandle m_backbufferHandle;

		std::vector<RenderableObject> m_renderables;
		ViewObject m_viewObject;
	};
}