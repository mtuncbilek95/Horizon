#pragma once

#include <Runtime/Reflection/Type.h>
#include <Runtime/Reflection/TypeAttribute.h>

#include <string_view>

namespace Horizon
{
	class MainMenuAttribute : public TypeAttribute
	{
	public:
		MainMenuAttribute(std::string_view path, usize order) : m_path(path),
			m_order(order)
		{
		}

		std::string_view GetPath() const { return m_path; }
		usize GetOrder() const { return m_order; }

		ReflectionTypeHandle GetTypeId() const final { return TypeIdOf<MainMenuAttribute>(); }

	private:
		std::string_view m_path;
		usize m_order = 0;
	};

	class MenuItemAttribute : public TypeAttribute
	{
	public:
		MenuItemAttribute(std::string_view path, usize order) : m_path(path), m_order(order)
		{
		}

		std::string_view GetPath() const { return m_path; }
		usize GetOrder() const { return m_order; }

		ReflectionTypeHandle GetTypeId() const final { return TypeIdOf<MenuItemAttribute>(); }

	private:
		std::string_view m_path;
		usize m_order = 0;
	};
}