#pragma once

#include <Runtime/Containers/List.h>
#include <Runtime/Containers/ReadOnlyList.h>
#include <Runtime/Definitions/Allocator.h>
#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/RTTR/Attribute.h>
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
		~Field()
		{
			for (Attribute* pAttr : m_attributes)
				Memory::Allocator::Delete(pAttr);
		}

		Field(const Field&) = delete;
		Field& operator=(const Field&) = delete;

		Field(Field&&) noexcept = default;
		Field& operator=(Field&&) noexcept = default;

		const std::string& GetName() const { return m_name; }
		usize GetOffset() const { return m_offset; }

		TypeKind GetKind() const { return m_kind; }
		TypeMode GetMode() const { return m_mode; }
		TypeHandle GetTypeId() const { return m_typeId; }
		TypeKind GetUnderlyingKind() const { return m_underlyingKind; }

		ReadOnlyList<Attribute* const> GetAttributes() const { return m_attributes; }

		template<typename TAttr>
		TAttr* GetCustomAttribute() const
		{
			for (Attribute* pAttr : m_attributes)
			{
				if (pAttr->GetTypeId() == TypeOf<TAttr>())
					return static_cast<TAttr*>(pAttr);
			}

			return nullptr;
		}

		template<typename TAttr>
		List<TAttr*> GetCustomAttributes() const
		{
			List<TAttr*> out;
			for (Attribute* pAttr : m_attributes)
			{
				if (pAttr->GetTypeId() == TypeOf<TAttr>())
					out.PushBack(static_cast<TAttr*>(pAttr));
			}

			return out;
		}

		void* GetValue(void* pInstance) const
		{
			return static_cast<c8*>(pInstance) + m_offset;
		}

		const void* GetValue(const void* pInstance) const
		{
			return static_cast<const c8*>(pInstance) + m_offset;
		}

		template<typename T>
		T& GetValueAs(void* pInstance) const
		{
			return *reinterpret_cast<T*>(static_cast<c8*>(pInstance) + m_offset);
		}

		template<typename T>
		const T& GetValueAs(const void* pInstance) const
		{
			return *reinterpret_cast<const T*>(static_cast<const c8*>(pInstance) + m_offset);
		}

		template<typename T>
		void SetValue(void* pInstance, const T& value) const
		{
			*reinterpret_cast<T*>(static_cast<c8*>(pInstance) + m_offset) = value;
		}

	private:
		std::string m_name;
		usize m_offset = 0;
		TypeKind m_kind = TypeKind::Object;
		TypeKind m_underlyingKind = TypeKind::Object;
		TypeMode m_mode = TypeMode::Invalid;

		TypeHandle m_typeId;

		List<Attribute*> m_attributes;
	};
}