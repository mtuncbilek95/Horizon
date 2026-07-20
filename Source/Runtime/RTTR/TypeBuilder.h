#pragma once

#include <Runtime/RTTR/Attribute.h>
#include <Runtime/RTTR/EnumValue.h>
#include <Runtime/RTTR/Field.h>
#include <Runtime/RTTR/Type.h>
#include <Runtime/RTTR/TypeHandle.h>
#include <Runtime/RTTR/TypeKind.h>
#include <Runtime/RTTR/TypeMode.h>
#include <Runtime/RTTR/TypeResolve.h>

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

			if constexpr (std::is_class_v<TType> && std::is_default_constructible_v<TType> &&
				!std::is_abstract_v<TType>)
			{
				builder.m_type.CreateMemoryFunc = []() -> VoidObject
					{
						return Allocator::Create<TType>(CurrLoc());
					};
				builder.m_type.DestroyMemoryFunc = [](VoidObject instance)
					{
						Allocator::Delete(static_cast<TType*>(instance));
					};
			}
			else if constexpr (std::is_abstract_v<TType>)
			{
				builder.m_type.m_abstractClass = true;
			}

			return builder;
		}

		template<typename TBase>
		TypeBuilder& WithBase()
		{
			m_type.m_baseId = TypeOf<TBase>();
			return *this;
		}

		template<typename TAttr, typename... Args>
		TypeBuilder& WithAttribute(Args&&... args)
		{
			m_type.m_attributes.push_back(Allocator::Create<TAttr>(CurrLoc(), std::forward<Args>(args)...));
			return *this;
		}

		TypeBuilder& WithEnum(const std::string& name, i64 value)
		{
			m_type.m_enumValues.push_back({ name, value });
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
			field.m_mode = R::Mode;
			field.m_typeId = TypeOf<typename R::Element>();

			if constexpr (R::Mode == TypeMode::Array)
				field.m_arrayOps = VectorOpsFor<typename R::Element>();

			m_type.m_fields.push_back(std::move(field));
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