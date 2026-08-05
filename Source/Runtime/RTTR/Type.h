#pragma once

#include <Runtime/Definitions/Allocator.h>
#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/RTTR/EnumValue.h>
#include <Runtime/RTTR/Field.h>
#include <Runtime/RTTR/TypeKind.h>
#include <Runtime/RTTR/TypeMode.h>

#include <Runtime/Containers/List.h>

#include <string>
#include <span>

namespace Horizon::Reflect
{
	using VoidObject = void*;

	class H_EXPORT Type
	{
		template<typename> 
		friend class TypeBuilder;
	public:
		Type() = default;
		~Type()
		{
			for (Attribute* attr : m_attributes)
				Allocator::Delete(attr);
		}

		Type(const Type&) = delete;
		Type& operator=(const Type&) = delete;

		Type(Type&&) noexcept = default;
		Type& operator=(Type&&) noexcept = default;

		VoidObject CreateFromMemory() const { return CreateMemoryFunc(); }
		void DestroyFromMemory(VoidObject pInstance) const { return DestroyMemoryFunc(pInstance); }

		TypeHandle GetTypeId() const { return m_typeId; }
		TypeHandle GetBaseId() const { return m_baseId; }

		const std::string& GetName() const { return m_name; }
		usize GetSizeInBytes() const { return m_size; }
		usize GetAlignment() const { return m_align; }
		b8 GetIsAbstract() const { return m_abstractClass; }

		std::span<Attribute* const> GetAttributes() const { return { m_attributes.GetData(), m_attributes.GetCount() }; }
		std::span<const EnumValue> GetEnumValues() const { return { m_enumValues.GetData(), m_enumValues.GetCount() }; }
		std::span<const Field> GetFields() const { return { m_fields.GetData(), m_fields.GetCount() }; }

		template<typename TAttr>
		TAttr* GetCustomAttribute()
		{
			for (Attribute* attr : m_attributes)
			{
				if (attr->GetTypeId() == TypeOf<TAttr>())
					return static_cast<TAttr*>(attr);
			}

			return nullptr;
		}

		template<typename TAttr>
		List<TAttr*> GetCustomAttributes()
		{
			List<TAttr*> out;
			for (Attribute* attr : m_attributes)
			{
				if (attr->GetTypeId() == TypeOf<TAttr>())
					out.PushBack(static_cast<TAttr*>(attr));
			}

			return out;
		}

	private:
		VoidObject(*CreateMemoryFunc)() = nullptr;
		void(*DestroyMemoryFunc)(VoidObject) = nullptr;

		TypeHandle m_typeId;
		TypeHandle m_baseId;

		std::string m_name;
		usize m_size = 0;
		usize m_align = 0;
		TypeKind m_kind = TypeKind::Object;
		b8 m_abstractClass = false;

		List<Attribute*> m_attributes;
		List<EnumValue> m_enumValues;
		List<Field> m_fields;
	};
}