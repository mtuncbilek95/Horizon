#pragma once

#include <Runtime/Reflection/Type.h>

namespace Horizon
{
	template<typename T>
	class ModuleAddressDispatcher
	{
	public:
		static void SetModuleAddress(void* pAddress)
		{
			TypeAccessor<T>::SetAddress(pAddress);
		}

		static Type** GetModuleAddress()
		{
			return TypeAccessor<T>::GetModuleAddress();
		}

	public:
		ModuleAddressDispatcher() = delete;
		~ModuleAddressDispatcher() = delete;
	};
}