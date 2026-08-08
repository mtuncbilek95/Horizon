#pragma once

#include <Runtime/RTTR/Reflection.h>

#include <string>
#include <string_view>

namespace Horizon::Editor
{
	class H_EXPORT MenuItemAttribute : public Reflect::Attribute
	{
	public:
		MenuItemAttribute(std::string path, i32 order, b8 isCheckbox = false) : m_path(path), m_order(order), m_isCheckbox(isCheckbox)
		{}
		~MenuItemAttribute() = default;

		const std::string& GetPath() const { return m_path; }
		i32 GetOrder() const { return m_order; }
		b8 GetIsCheckbox() const { return m_isCheckbox; }

		Reflect::TypeHandle GetTypeId() const final { return Reflect::TypeOf<MenuItemAttribute>(); }

	private:
		std::string m_path;
		i32 m_order;
		b8 m_isCheckbox;
	};
}