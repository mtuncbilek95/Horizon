#pragma once

#include <Engine/Core/Context.h>

namespace Horizon
{
	class GfxPipeline;

	// Just a basic test system
	class DummyContext : public Context<DummyContext>
	{
	public:
		GfxPipeline* CompositePipeline() const { return m_compositePipeline.get(); }

	private:
		EngineReport OnInitialize() final;
		void OnFinalize() final;

	private:
		std::shared_ptr<GfxPipeline> m_compositePipeline;
	};
}