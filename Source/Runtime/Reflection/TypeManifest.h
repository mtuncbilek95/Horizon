#pragma once

#include <Runtime/Reflection/Type.h>
#include <Runtime/Reflection/TypeAttribute.h>
#include <Runtime/Reflection/FieldManifest.h>

namespace Horizon
{
	class TypeManifest
	{
		friend class TypeManifestBuilder;
	public:
		TypeManifest() = default;

		~TypeManifest()
		{
			for (TypeAttribute* attr : m_attributes)
				Allocator::Delete(attr);
		}

		TypeManifest(const TypeManifest&) = delete;
		TypeManifest& operator=(const TypeManifest&) = delete;

		TypeManifest(TypeManifest&& other) noexcept { MoveFrom(std::move(other)); }

		TypeManifest& operator=(TypeManifest&& other) noexcept
		{
			if (this != &other)
			{
				for (TypeAttribute* attr : m_attributes)
					Allocator::Delete(attr);

				MoveFrom(std::move(other));
			}
			return *this;
		}

		ReflectionTypeHandle GetTypeId() const { return m_typeId; }
		std::string_view GetName() const { return m_name; }

		usize GetSize() const { return m_size; }
		usize GetAlignment() const { return m_align; }

		b8 IsAbstract() const { return m_createFunc == nullptr; }

		void* Create() const { return m_createFunc ? m_createFunc() : nullptr; }
		void Destroy(void* obj) const { if (m_destroyFunc) m_destroyFunc(obj); }

		ReflectionTypeHandle GetBaseTypeId() const { return m_baseTypeId; }

		const std::vector<TypeAttribute*>& GetAttributes() const { return m_attributes; }
		const std::vector<FieldManifest>& GetFields() const { return m_fields; }

		template<typename TAttr>
		TAttr* GetCustomAttribute()
		{
			for (TypeAttribute* attr : m_attributes)
			{
				if (attr->GetTypeId() == TypeIdOf<TAttr>())
					return static_cast<TAttr*>(attr);
			}

			return nullptr;
		}

		template<typename TAttr>
		std::vector<TAttr*> GetCustomAttributes()
		{
			std::vector<TAttr*> out;
			for (TypeAttribute* attr : m_attributes)
			{
				if (attr->GetTypeId() == TypeIdOf<TAttr>())
					out.push_back(static_cast<TAttr*>(attr));
			}

			return out;
		}

	private:
		void MoveFrom(TypeManifest&& o) noexcept
		{
			m_typeId = o.m_typeId;
			m_name = o.m_name;
			m_size = o.m_size;
			m_align = o.m_align;
			m_baseTypeId = o.m_baseTypeId;
			m_createFunc = o.m_createFunc;
			m_destroyFunc = o.m_destroyFunc;
			m_attributes = std::move(o.m_attributes);
		}

	private:
		ReflectionTypeHandle m_typeId;
		std::string_view m_name;
		usize m_size = 0;
		usize m_align = 0;
		ReflectionTypeHandle m_baseTypeId;

		void* (*m_createFunc)() = nullptr;
		void (*m_destroyFunc)(void*) = nullptr;

		std::vector<TypeAttribute*> m_attributes;
		std::vector<FieldManifest> m_fields;
	};
}