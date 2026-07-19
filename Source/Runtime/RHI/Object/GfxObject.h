#pragma once

namespace Horizon
{
	class GfxDevice;

	class GfxObject
	{
		friend class GfxDevice;

	public:
		virtual ~GfxObject() = default;

		virtual void SetDebugName(const char* pName) {}

		GfxDevice* GetOwnerDevice() const { return m_ownerDevice; }

	protected:
		GfxDevice* m_ownerDevice = nullptr;
	};
}