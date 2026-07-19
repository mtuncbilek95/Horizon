#pragma once

#include <Runtime/RTTR/Reflection.h>

#include <string_view>

namespace Horizon
{
	class MainMenuAttribute : public Reflect::Attribute
	{
	public:
		MainMenuAttribute(std::string_view path, usize order) : m_path(path),
			m_order(order)
		{
		}

		std::string_view GetPath() const { return m_path; }
		usize GetOrder() const { return m_order; }

		Reflect::TypeHandle GetTypeId() const final { return Reflect::TypeOf<MainMenuAttribute>(); }

	private:
		std::string_view m_path;
		usize m_order = 0;
	};

	class MenuItemAttribute : public Reflect::Attribute
	{
	public:
		MenuItemAttribute(std::string_view path, usize order) : m_path(path), m_order(order)
		{
		}

		std::string_view GetPath() const { return m_path; }
		usize GetOrder() const { return m_order; }

		Reflect::TypeHandle GetTypeId() const final { return Reflect::TypeOf<MenuItemAttribute>(); }

	private:
		std::string_view m_path;
		usize m_order = 0;
	};
}