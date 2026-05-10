#pragma once

#include <Engine/Core/Context.h>
#include <Engine/Cache/WindowCache.h>
#include <Engine/Cache/GfxCache.h>

namespace Horizon
{
	class CacheContext : public Context<CacheContext>
	{
	public:
		WindowCache& GetWindowCache() { return m_windowCache; }
		GfxCache& GetGfxCache() { return m_gfxCache; }

	private:
		EngineReport OnInitialize() final;
		void OnFinalize() final;

	private:
		WindowCache m_windowCache;
		GfxCache m_gfxCache;
	};
}