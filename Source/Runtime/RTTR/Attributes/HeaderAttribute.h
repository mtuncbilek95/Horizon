#pragma once

#include <Runtime/RTTR/Reflection.h>

#include <string>
#include <string_view>

namespace Horizon::Editor
{
	class H_EXPORT HeaderAttribute : public Reflect::Attribute
	{
		HORIZON_ATTRIBUTE_REFLECT(HeaderAttribute);
	public:
		HeaderAttribute(const std::string& header) : m_header(header) 
		{
		}
		~HeaderAttribute() = default;

		const std::string& GetHeaderName() const { return m_header; }

	private:
		std::string m_header;
	};
}