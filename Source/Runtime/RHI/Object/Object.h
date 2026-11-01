#pragma once

namespace Horizon::RHI
{
	class Device;

	class RUNTIME_API Object
	{
	public:
		virtual ~Object() = default;
		
		virtual void SetDebugName(const char* pName) {}
		
		Device* GetOwnerDevice() const { return m_ownerDevice; }

	protected:
		Device* m_ownerDevice = nullptr;
	};
}