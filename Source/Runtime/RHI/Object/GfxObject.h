#pragma once

namespace Horizon::RHI
{
	class GfxDevice;

	class RUNTIME_API GfxObject
	{
	public:
		virtual ~GfxObject() = default;
		virtual void SetDebugName(const char* pName) {}
		GfxDevice* GetOwnerDevice() const { return m_ownerDevice; }
	protected:
		GfxDevice* m_ownerDevice = nullptr;
	};
}