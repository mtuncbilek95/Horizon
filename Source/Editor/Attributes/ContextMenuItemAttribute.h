#pragma once

#include <Runtime/RTTR/Reflection.h>

#include <string>

namespace Horizon
{
	class H_EXPORT ContextMenuItemAttribute : public Reflect::Attribute
	{
	public:
		ContextMenuItemAttribute(std::string path, i32 order) : m_path(path), m_order(order)
		{
		}
		~ContextMenuItemAttribute() = default;

		const std::string& GetPath() const { return m_path; }
		i32 GetOrder() const { return m_order; }

		Reflect::TypeHandle GetTypeId() const final { return Reflect::TypeOf<ContextMenuItemAttribute>(); }

	private:
		std::string m_path;
		i32 m_order;
	};
}