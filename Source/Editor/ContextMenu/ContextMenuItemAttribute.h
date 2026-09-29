#pragma once

#include <Runtime/RTTR/Reflection.h>

#include <string>

namespace Horizon::Editor
{
	class H_EXPORT ContextMenuItemAttribute : public Reflect::Attribute
	{
		HORIZON_ATTRIBUTE_REFLECT(ContextMenuItemAttribute);
	public:
		ContextMenuItemAttribute(const std::string& owner, const std::string& path, i32 order)
			: m_owner(owner), m_path(path), m_order(order)
		{
		}
		~ContextMenuItemAttribute() = default;

		const std::string& GetOwner() const { return m_owner; }
		const std::string& GetPath() const { return m_path; }
		i32 GetOrder() const { return m_order; }

	private:
		std::string m_owner;
		std::string m_path;
		i32 m_order;
	};
}