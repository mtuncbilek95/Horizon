#pragma once

#include <Engine/Core/Engine.h>
#include <Runtime/RHI/Device/GfxDevice.h>

namespace Horizon::Engine
{
	class MeshResourceCache
	{
		friend class MeshAssetStreamer;
	public:
		MeshResourceCache() = default;
		~MeshResourceCache() = default;

	private:
		Engine* m_engine = nullptr;
		RHI::GfxDevice* m_device = nullptr;
	};
}