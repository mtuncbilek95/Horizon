#pragma once

#include <Runtime/Definitions/Allocator.h>
#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/Log/Terminal.h>
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

	class H_EXPORT Type final
	{
		template<typename>
		friend class TypeBuilder;
	public:
		Type() = default;
		~Type()
		{
			for (Attribute* attr : m_attributes)
				Memory::Allocator::Delete(attr);
		}

		Type(const Type&) = delete;
		Type& operator=(const Type&) = delete;

		Type(Type&&) noexcept = default;
		Type& operator=(Type&&) noexcept = default;

		TypeHandle GetTypeId() const { return m_typeId; }
		TypeHandle GetBaseId() const { return m_baseId; }

		const std::string& GetName() const { return m_name; }
		TypeKind GetKind() const { return m_kind; }
		usize GetSizeInBytes() const { return m_size; }
		usize GetAlignment() const { return m_align; }

		b8 GetIsAbstract() const { return m_abstractClass; }
		b8 IsTriviallyCopyable() const { return m_triviallyCopyable; }

		b8 CanConstruct() const { return ConstructFunc != nullptr; }
		b8 CanMove() const { return MoveFunc != nullptr; }
		b8 CanCopy() const { return CopyFunc != nullptr; }

		void ConstructAt(void* pMemory) const
		{
			if (!ConstructFunc)
			{
				Terminal::Error("Type", "'{}' is not default constructible", m_name);
				return;
			}

			ConstructFunc(pMemory);
		}

		void DestructAt(void* pMemory) const
		{
			if (!DestructFunc)
				return;

			DestructFunc(pMemory);
		}

		void MoveAt(void* pDestination, void* pSource) const
		{
			if (!MoveFunc)
			{
				Terminal::Error("Type", "'{}' is not move constructible", m_name);
				return;
			}

			MoveFunc(pDestination, pSource);
		}

		void CopyAt(void* pDestination, const void* pSource) const
		{
			if (!CopyFunc)
			{
				Terminal::Error("Type", "'{}' is not copy constructible", m_name);
				return;
			}

			CopyFunc(pDestination, pSource);
		}

		VoidObject Create(Memory::SourceLocation loc = Memory::CurrLoc()) const
		{
			if (!ConstructFunc)
			{
				Terminal::Error("Type", "'{}' cannot be created", m_name);
				return nullptr;
			}

			void* pMemory = Memory::Allocator::AllocateRaw(m_size, m_align, loc);
			ConstructFunc(pMemory);

			return pMemory;
		}

		void Destroy(VoidObject pInstance) const
		{
			if (!pInstance)
				return;

			DestructAt(pInstance);
			Memory::Allocator::FreeRaw(pInstance);
		}

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
		using ConstructFn = void(*)(void*);
		using DestructFn = void(*)(void*);
		using MoveFn = void(*)(void*, void*);
		using CopyFn = void(*)(void*, const void*);

		ConstructFn ConstructFunc = nullptr;
		DestructFn DestructFunc = nullptr;
		MoveFn MoveFunc = nullptr;
		CopyFn CopyFunc = nullptr;

		TypeHandle m_typeId;
		TypeHandle m_baseId;

		std::string m_name;
		usize m_size = 0;
		usize m_align = 0;
		TypeKind m_kind = TypeKind::Object;
		b8 m_abstractClass = false;
		b8 m_triviallyCopyable = false;

		List<Attribute*> m_attributes;
		List<EnumValue> m_enumValues;
		List<Field> m_fields;
	};
}