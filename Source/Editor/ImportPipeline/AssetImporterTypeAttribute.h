#pragma once

#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Editor
{
	class EDITOR_API AssetImporterTypeAttribute : public Reflect::Attribute
	{
		HORIZON_ATTRIBUTE_REFLECT(AssetImporterTypeAttribute);
	public:
		AssetImporterTypeAttribute(Reflect::TypeHandle handl) : m_assetType(handl)
		{
		}
		~AssetImporterTypeAttribute() = default;

		Reflect::TypeHandle GetAssetType() const { return m_assetType; }

	private:
		Reflect::TypeHandle m_assetType;
	};
}