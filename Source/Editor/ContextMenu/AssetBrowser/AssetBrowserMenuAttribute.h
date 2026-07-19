#pragma once

#include <Runtime/RTTR/Reflection.h>

#include <string_view>

namespace Horizon
{
	class AssetBrowserMainAttribute : public Reflect::Attribute
	{
	public:
		AssetBrowserMainAttribute(std::string_view path, usize order) : m_path(path),
			m_order(order)
		{
		}

		std::string_view GetPath() const { return m_path; }
		usize GetOrder() const { return m_order; }

		Reflect::TypeHandle GetTypeId() const final { return Reflect::TypeOf<AssetBrowserMainAttribute>(); }

	private:
		std::string_view m_path;
		usize m_order = 0;
	};

	class AssetBrowserMenuItemAttribute : public Reflect::Attribute
	{
	public:
		AssetBrowserMenuItemAttribute(std::string_view path, usize order) : m_path(path), m_order(order)
		{
		}

		std::string_view GetPath() const { return m_path; }
		usize GetOrder() const { return m_order; }

		Reflect::TypeHandle GetTypeId() const final { return Reflect::TypeOf<AssetBrowserMenuItemAttribute>(); }

	private:
		std::string_view m_path;
		usize m_order = 0;
	};
}