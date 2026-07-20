#pragma once

#include <Runtime/RTTR/Reflection.h>

#include <string_view>

namespace Horizon
{
	class ImportStageAttribute : public Reflect::Attribute
	{
	public:
		ImportStageAttribute(std::string_view extension, usize order)
			: m_extension(extension), m_order(order)
		{
		}

		std::string_view GetExtension() const { return m_extension; }
		usize GetOrder() const { return m_order; }

		Reflect::TypeHandle GetTypeId() const final { return Reflect::TypeOf<ImportStageAttribute>(); }

	private:
		std::string_view m_extension;
		usize m_order = 0;
	};
}