#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/Input/DeviceType.h>
#include <Runtime/Input/ProductType.h>
#include <Runtime/Input/VendorType.h>

namespace Horizon::Input
{
	class Instance;
	using ObjectHandle = void*;

	class RUNTIME_API Object
	{
	public:
		Object(Instance* pInstance, ObjectHandle handle, DeviceType type, VendorType vendor, ProductType product)
			: m_instance(pInstance), m_handle(handle), m_type(type), m_vendor(vendor), m_product(product)
		{
		}
		virtual ~Object() = default;

		Instance* GetInstance() const { return m_instance; }
		ObjectHandle GetHandle() const { return m_handle; }
		DeviceType GetDeviceType() const { return m_type; }
		VendorType GetVendor() const { return m_vendor; }
		ProductType GetProduct() const { return m_product; }

	private:
		Instance* m_instance = nullptr;
		ObjectHandle m_handle = nullptr;
		DeviceType m_type = DeviceType::Unknown;
		VendorType m_vendor = VendorType::Unknown;
		ProductType m_product = ProductType::General;
	};
}