#pragma once

#include <Runtime/Reflection/Type.h>
#include <Runtime/Reflection/TypeAttribute.h>
#include <Runtime/Reflection/TypeManifest.h>

namespace Horizon
{
	class TypeManifestBuilder
	{
	public:
		template<typename T>
		static TypeManifestBuilder For(std::string_view name)
		{
			TypeManifestBuilder builder;

			builder.m_manifest.m_typeId = TypeIdOf<T>();
			builder.m_manifest.m_name = name;
			builder.m_manifest.m_size = sizeof(T);
			builder.m_manifest.m_align = alignof(T);

			if constexpr (std::is_default_constructible_v<T> && !std::is_abstract_v<T>)
			{
				builder.m_manifest.m_createFunc = []() -> void* { return Allocator::Create<T>(CurrLoc()); };
				builder.m_manifest.m_destroyFunc = [](void* p) { Allocator::Delete(static_cast<T*>(p)); };
			}

			return builder;
		}

		TypeManifestBuilder& WithBase(ReflectionTypeHandle baseId)
		{
			m_manifest.m_baseTypeId = baseId;
			return *this;
		}

		template<typename TAttr, typename... Args>
		TypeManifestBuilder& WithAttribute(Args&&... args)
		{
			m_manifest.m_attributes.push_back(
				Allocator::Create<TAttr>(CurrLoc(), std::forward<Args>(args)...));
			return *this;
		}

		TypeManifest Build() { return std::move(m_manifest); }

	private:
		TypeManifest m_manifest;
	};
}