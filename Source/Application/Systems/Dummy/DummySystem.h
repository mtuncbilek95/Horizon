#pragma once

#include <Application/Engine/System.h>

#include <memory>

namespace Horizon
{
	class GfxShader;
	class GfxPipeline;
	class GfxCommandPool;

	class RenderSystem;

	class DummySystem final : public System<DummySystem>
	{
	public:
		DummySystem() = default;
		~DummySystem() = default;

	private:
		EngineReport OnInitialize() final;
		void OnSync() final;
		void OnFinalize() final;

	private:
		RenderSystem* m_RS;
		std::shared_ptr<GfxPipeline> m_dummyTestPipeline;
		std::shared_ptr<GfxShader> m_vertShader;
		std::shared_ptr<GfxShader> m_fragShader;
		std::shared_ptr<GfxCommandPool> m_batchPool;
	};
}