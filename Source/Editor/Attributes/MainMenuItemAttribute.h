#pragma once

#include <Runtime/RTTR/Reflection.h>

#include <string>
#include <string_view>

namespace Horizon::Editor
{
	class H_EXPORT MainMenuItemAttribute : public Reflect::Attribute
	{
	public:
		MainMenuItemAttribute(std::string path, i32 order) : m_path(path), m_order(order)
		{
		}
		~MainMenuItemAttribute() = default;

		const std::string& GetPath() const { return m_path; }
		i32 GetOrder() const { return m_order; }

		Reflect::TypeHandle GetTypeId() const final { return Reflect::TypeOf<MainMenuItemAttribute>(); }

	private:
		std::string m_path;
		i32 m_order;
	};
}