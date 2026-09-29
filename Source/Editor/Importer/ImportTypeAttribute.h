#pragma once

#include <Runtime/RTTR/Reflection.h>
#include <Runtime/Containers/List.h>
#include <string>

namespace Horizon::Editor
{
	class H_EXPORT ImportTypeAttribute : public Reflect::Attribute
	{
		HORIZON_ATTRIBUTE_REFLECT(ImportTypeAttribute);
	public:
		ImportTypeAttribute(Reflect::TypeHandle handle, const List<std::string>& extensions) : m_workingType(handle),
			m_extensions(extensions)
		{}
		~ImportTypeAttribute() = default;

		Reflect::TypeHandle GetType() const { return m_workingType; }
		const List<std::string>& GetExtensions() const { return m_extensions; }

	private:
		Reflect::TypeHandle m_workingType;
		List<std::string> m_extensions;
	};
}