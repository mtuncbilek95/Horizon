#pragma once

#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Editor
{
	class H_EXPORT AssetOpenerAttribute : public Reflect::Attribute
	{
		HORIZON_ATTRIBUTE_REFLECT(AssetOpenerAttribute);
	public:
		AssetOpenerAttribute(Reflect::TypeHandle assetHandle) : m_assetHandle(assetHandle)
		{
		}
		~AssetOpenerAttribute() = default;

		Reflect::TypeHandle GetAssetHandle() const { return m_assetHandle; }

	private:
		Reflect::TypeHandle m_assetHandle;
	};
}