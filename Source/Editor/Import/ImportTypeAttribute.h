#pragma once

#include <Runtime/Containers/List.h>
#include <Runtime/RTTR/Reflection.h>
#include <string>

namespace Horizon::Editor
{
	class H_EXPORT ImportTypeAttribute final : public Reflect::Attribute
	{
		HORIZON_ATTRIBUTE_REFLECT(ImportTypeAttribute);
	public:
		ImportTypeAttribute(Reflect::TypeHandle assetHandle, const List<std::string>& extensions) :
			m_assetHandle(assetHandle), m_extensions(extensions)
		{
		}

		~ImportTypeAttribute() = default;

		Reflect::TypeHandle GetAssetHandle() const { return m_assetHandle; }
		const List<std::string>& GetExtensions() const { return m_extensions; }

	private:
		Reflect::TypeHandle m_assetHandle;
		List<std::string> m_extensions;
	};
}