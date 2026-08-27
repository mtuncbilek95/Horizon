#pragma once

#include <Runtime/RTTR/Attribute.h>
#include <Runtime/RTTR/EnumValue.h>
#include <Runtime/RTTR/Field.h>
#include <Runtime/RTTR/Type.h>
#include <Runtime/RTTR/TypeHandle.h>
#include <Runtime/RTTR/TypeKind.h>
#include <Runtime/RTTR/TypeMode.h>
#include <Runtime/RTTR/TypeResolve.h>

#include <Runtime/Log/Terminal.h>

namespace Horizon::Reflect
{
	template<typename TType>
	class TypeBuilder
	{
	public:
		static TypeBuilder ForType(std::string name)
		{
			TypeBuilder builder = {};
			builder.m_type.m_typeId = TypeOf<TType>();
			builder.m_type.m_name = std::move(name);
			builder.m_type.m_size = sizeof(TType);
			builder.m_type.m_align = alignof(TType);
			builder.m_type.m_kind = KindOf<TType>();
			builder.m_type.m_abstractClass = std::is_abstract_v<TType>;
			builder.m_type.m_triviallyCopyable = std::is_trivially_copyable_v<TType>;

			if constexpr (!std::is_abstract_v<TType>)
			{
				builder.m_type.DestructFunc = [](void* pMemory)
					{
						static_cast<TType*>(pMemory)->~TType();
					};

				if constexpr (std::is_default_constructible_v<TType>)
				{
					builder.m_type.ConstructFunc = [](void* pMemory)
						{
							::new (pMemory) TType();
						};
				}

				if constexpr (std::is_move_constructible_v<TType>)
				{
					builder.m_type.MoveFunc = [](void* pDestination, void* pSource)
						{
							::new (pDestination) TType(std::move(*static_cast<TType*>(pSource)));
						};
				}

				if constexpr (std::is_copy_constructible_v<TType>)
				{
					builder.m_type.CopyFunc = [](void* pDestination, const void* pSource)
						{
							::new (pDestination) TType(*static_cast<const TType*>(pSource));
						};
				}
			}

			return builder;
		}

		template<typename TBase>
		TypeBuilder& WithBase()
		{
			m_type.m_baseId = TypeOf<TBase>();
			return *this;
		}

		template<typename TAttr>
		TypeBuilder& WithAttribute(TAttr attr)
		{
			m_type.m_attributes.PushBack(Memory::Allocator::Create<TAttr>(Memory::CurrLoc(), std::move(attr)));
			return *this;
		}

		TypeBuilder& WithEnum(const std::string& name, i64 value)
		{
			m_type.m_enumValues.PushBack({ name, value });
			return *this;
		}

		template<typename TOwner, typename TField>
		TypeBuilder& WithField(std::string name, TField TOwner::* member)
		{
			using R = TypeResolve<TField>;

			Field field;
			field.m_name = std::move(name);
			field.m_offset = OffsetOf(member);
			field.m_kind = R::Kind;
			field.m_underlyingKind = UnderlyingKindOf<typename R::Element>();
			field.m_mode = R::Mode;
			field.m_typeId = TypeOf<typename R::Element>();

			m_type.m_fields.PushBack(std::move(field));
			return *this;
		}

		template<typename TAttr>
		TypeBuilder& WithFieldAttribute(TAttr attr)
		{
			if (m_type.m_fields.IsEmpty())
			{
				Terminal::Error("TypeBuilder", "WithFieldAttribute called before any WithField on '{}'", m_type.m_name);
				return *this;
			}

			m_type.m_fields.Back().m_attributes.PushBack(
				Memory::Allocator::Create<TAttr>(Memory::CurrLoc(), std::move(attr)));
			return *this;
		}

		Type Build() { return std::move(m_type); }

	private:
		template<typename TOwner, typename TField>
		static usize OffsetOf(TField TOwner::* member)
		{
			return reinterpret_cast<usize>(&(reinterpret_cast<const TOwner*>(0)->*member));
		}

	private:
		Type m_type;
	};
}