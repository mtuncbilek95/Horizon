#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::RHI
{
	class Device;

	class Object
	{
	public:
		virtual ~Object() = default;
		
		virtual void SetDebugName(const c8* pName) {}
		
		Device* GetOwnerDevice() const { return m_ownerDevice; }

	protected:
		Device* m_ownerDevice = nullptr;
	};
}