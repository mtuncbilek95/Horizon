#pragma once

#include <Runtime/Containers/List.h>
#include <Runtime/Definitions/Allocator.h>
#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/RTTR/Attribute.h>
#include <Runtime/RTTR/TypeKind.h>
#include <Runtime/RTTR/TypeMode.h>

#include <string>
#include <span>

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
			for (Attribute* attr : m_attributes)
				Memory::Allocator::Delete(attr);
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

		std::span<Attribute* const> GetAttributes() const { return { m_attributes.GetData(), m_attributes.GetCount() }; }

		template<typename TAttr>
		TAttr* GetCustomAttribute() const
		{
			for (Attribute* attr : m_attributes)
			{
				if (attr->GetTypeId() == TypeOf<TAttr>())
					return static_cast<TAttr*>(attr);
			}

			return nullptr;
		}

		template<typename TAttr>
		List<TAttr*> GetCustomAttributes() const
		{
			List<TAttr*> out;
			for (Attribute* attr : m_attributes)
			{
				if (attr->GetTypeId() == TypeOf<TAttr>())
					out.PushBack(static_cast<TAttr*>(attr));
			}

			return out;
		}

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
		TypeKind m_underlyingKind = TypeKind::Object;
		TypeMode m_mode = TypeMode::Invalid;

		TypeHandle m_typeId;

		List<Attribute*> m_attributes;
	};
}