#pragma once

#include <Runtime/RTTR/Reflection.h>

namespace Horizon
{
	class H_EXPORT ImportFileAttribute : public Reflect::Attribute
	{
	public:
		ImportFileAttribute(const Reflect::TypeHandle& handle) : m_handle(handle) {}
		~ImportFileAttribute() = default;

		Reflect::TypeHandle GetTypeId() const final { return Reflect::TypeOf<ImportFileAttribute>(); }
		Reflect::TypeHandle GetTypeHandle() const { return m_handle; }

	private:
		Reflect::TypeHandle m_handle;
	};
}