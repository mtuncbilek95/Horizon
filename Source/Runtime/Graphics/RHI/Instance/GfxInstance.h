#pragma once

#include <Runtime/Graphics/RHI/Device/GfxDeviceDesc.h>
#include <Runtime/Graphics/RHI/Instance/GfxInstanceDesc.h>

#include <memory>

namespace Horizon
{
	class GfxDevice;

	/**
	 * @class GfxInstance
	 * @brief Main instance and gpu device generator for RHI.
	 */
	class GfxInstance
	{
	public:
		static std::shared_ptr<GfxInstance> Create(const GfxInstanceDesc& desc);

	public:
		GfxInstance(const GfxInstanceDesc& desc); 
		virtual ~GfxInstance() = default;

		virtual std::shared_ptr<GfxDevice> CreateDevice(const GfxDeviceDesc& desc) = 0;
		virtual void* Instance() const = 0;
		virtual void* Adapter() const = 0;

	private:
		GfxInstanceDesc m_desc;
	};
}
