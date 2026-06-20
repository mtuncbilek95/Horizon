#pragma once

namespace Horizon
{
	class GfxDevice;

	class GfxResource
	{
		friend class GfxDevice;

	public:
		virtual ~GfxResource() = default;

		GfxDevice* GetOwnerDevice() const { return m_ownerDevice; }

	protected:
		GfxDevice* m_ownerDevice = nullptr;
	};
}
