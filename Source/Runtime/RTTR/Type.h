#pragma once

#include <Runtime/Definitions/Allocator.h>
#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/RTTR/EnumValue.h>
#include <Runtime/RTTR/Field.h>
#include <Runtime/RTTR/TypeKind.h>
#include <Runtime/RTTR/TypeMode.h>

#include <Runtime/Containers/List.h>
#include <Runtime/Containers/ReadOnlyList.h>

#include <string>

namespace Horizon::Reflect
{
	using VoidObject = void*;

	class RUNTIME_API Type final
	{
		template<typename>
		friend class TypeBuilder;
	public:
		Type() = default;
		~Type()
		{
			for (Attribute* pAttr : m_attributes)
				Memory::Allocator::Delete(pAttr);
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

		b8 CanConstruct() const { return m_constructFunc != nullptr; }
		b8 CanMove() const { return m_moveFunc != nullptr; }
		b8 CanCopy() const { return m_copyFunc != nullptr; }

		void ConstructAt(void* pMemory) const
		{
			if (!m_constructFunc)
			{
				Terminal::Error("Type", "'{}' is not default constructible", m_name);
				return;
			}

			m_constructFunc(pMemory);
		}

		void DestructAt(void* pMemory) const
		{
			if (!m_destructFunc)
				return;

			m_destructFunc(pMemory);
		}

		void MoveAt(void* pDestination, void* pSource) const
		{
			if (!m_moveFunc)
			{
				Terminal::Error("Type", "'{}' is not move constructible", m_name);
				return;
			}

			m_moveFunc(pDestination, pSource);
		}

		void CopyAt(void* pDestination, const void* pSource) const
		{
			if (!m_copyFunc)
			{
				Terminal::Error("Type", "'{}' is not copy constructible", m_name);
				return;
			}

			m_copyFunc(pDestination, pSource);
		}

		VoidObject Create(Memory::SourceLocation loc = Memory::CurrLoc()) const
		{
			if (!m_constructFunc)
			{
				Terminal::Error("Type", "'{}' cannot be created", m_name);
				return nullptr;
			}

			void* pMemory = Memory::Allocator::AllocateRaw(m_size, m_align, loc);
			m_constructFunc(pMemory);

			return pMemory;
		}

		void Destroy(VoidObject pInstance) const
		{
			if (!pInstance)
				return;

			DestructAt(pInstance);
			Memory::Allocator::FreeRaw(pInstance);
		}

		ReadOnlyList<Attribute* const> GetAttributes() const { return m_attributes; }
		ReadOnlyList<const EnumValue> GetEnumValues() const { return m_enumValues; }
		ReadOnlyList<const Field> GetFields() const { return m_fields; }

		template<typename TAttr>
		b8 HasCustomAttribute()
		{
			for (Attribute* pAttr : m_attributes)
			{
				if (pAttr->GetTypeId() == TypeOf<TAttr>())
					return true;
			}

			return false;
		}

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

	private:
		using ConstructFn = void(*)(void*);
		using DestructFn = void(*)(void*);
		using MoveFn = void(*)(void*, void*);
		using CopyFn = void(*)(void*, const void*);

		ConstructFn m_constructFunc = nullptr;
		DestructFn m_destructFunc = nullptr;
		MoveFn m_moveFunc = nullptr;
		CopyFn m_copyFunc = nullptr;

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