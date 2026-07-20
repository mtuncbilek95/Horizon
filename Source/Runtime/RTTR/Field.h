#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/RTTR/ArrayOps.h>
#include <Runtime/RTTR/TypeKind.h>
#include <Runtime/RTTR/TypeMode.h>

#include <string>

namespace Horizon::Reflect
{
	class Type;

	class H_EXPORT Field
	{
		template<typename> 
		friend class TypeBuilder;
	public:
		Field() = default;
		Field(const std::string& name, usize offset) : m_name(name),
			m_offset(offset)
		{
		}
		~Field() = default;

		const std::string& GetName() const { return m_name; }
		usize GetOffset() const { return m_offset; }

		TypeKind GetKind() const { return m_kind; }
		TypeMode GetMode() const { return m_mode; }

		TypeHandle GetTypeId() const { return m_typeId; }

		const ArrayOps* GetArrayOps() const { return m_arrayOps; }

		void* GetValue(void* instance) const
		{
			return static_cast<c8*>(instance) + m_offset;
		}

		const void* GetValue(const void* instance) const
		{
			return static_cast<const c8*>(instance) + m_offset;
		}

		template<typename T>
		T& GetValueAs(void* instance) const
		{
			return *reinterpret_cast<T*>(static_cast<c8*>(instance) + m_offset);
		}

		template<typename T>
		const T& GetValueAs(const void* instance) const
		{
			return *reinterpret_cast<const T*>(static_cast<const c8*>(instance) + m_offset);
		}

		template<typename T>
		void SetValue(void* instance, const T& value) const
		{
			*reinterpret_cast<T*>(static_cast<c8*>(instance) + m_offset) = value;
		}

	private:
		std::string m_name;
		usize m_offset = 0;
		TypeKind m_kind = TypeKind::Object;
		TypeMode m_mode = TypeMode::Compose;

		TypeHandle m_typeId;

		const ArrayOps* m_arrayOps = nullptr;
	};
}