#pragma once

#include <Runtime/RTTR/Reflection.h>

#include <string>
#include <string_view>

namespace Horizon::Editor
{
	class H_EXPORT HeaderAttribute : public Reflect::Attribute
	{
	public:
		HeaderAttribute(const std::string& header) : m_header(header) 
		{
		}
		~HeaderAttribute() = default;

		const std::string& GetHeaderName() const { return m_header; }

		Reflect::TypeHandle GetTypeId() const final { return Reflect::TypeOf<HeaderAttribute>(); }

	private:
		std::string m_header;
	};
}