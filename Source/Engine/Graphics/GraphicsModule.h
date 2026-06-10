#pragma once

#include <Engine/Engine/IModule.h>
#include <Runtime/Graphics/GfxBackend.h>

#include <string>
#include <vector>

namespace Horizon
{
	/*
	 * Stores the texture, buffer, pipeline pools.
	 */
	class GraphicsModule : public IModule
	{
	public:
		GraphicsModule();
		~GraphicsModule();

		void OnAttach(Engine& engine);
		void OnDetach();

	private:
		GfxDevice* m_mainDevice = nullptr;
		GfxSwapchain* m_swapchain = nullptr;

		// This should be here so Memory management can be done in one more layer up.
		// ObjectSlotMap<OpaqueStruct, GfxTextureHandle, maxCount> m_texturePool;
	};
}