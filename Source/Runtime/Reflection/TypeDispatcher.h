#pragma once

#include <Runtime/Reflection/Type.h>
#include <Runtime/Reflection/Field.h>
#include <Runtime/Reflection/FieldMode.h>

#include <Runtime/Definitions/Allocator.h>

namespace Horizon
{
	class H_EXPORT TypeDispatcher final
	{
	public:
		static Type* CreateType(const String& name, const String& namespaces, const u32 sizeInBytes, 
			const TypeModes mode, const TypeCodes code, const DefaultHeapObjectGenerator generator, Type** ppAddress);
		static void DeleteType(Type* pType);
		static void DeleteField(Field* pField);
		static void RegisterEnum(const String& name, const i64 value, Type* pTargetType);
		static void RegisterField(const String& name, const u32 offset, Type* pFieldType, const FieldMode mode, Type* pTargetType);
		static void RegisterAttribute(Type* pTargetType, Attribute* pAttribute);
		static void SetBaseType(Type* pTargetType, Type* pBaseType);
		static void IncrementTypeShare(Type* pTargetType);
		static void DecrementTypeShare(Type* pTargetType);
		static Type** GetTypeAddress(const Type* pType);

		template<typename T>
		static Type** GetTypeAddress()
		{
			return TypeAccessor<T>::GetTypeAddress();
		}

		template<typename T>
		static void SetTypeAddress(Type* pType)
		{
			TypeAccessor<T>::SetType(pType);
		}

		template<typename T, typename... TArgs>
		static void RegisterAttribute(Type* pTargetType, TArgs... args)
		{
			T* pAttribute = Allocator::Create<T>(CurrLoc(), args...);
			pTargetType->RegisterAttribute(pAttribute);
		}

		template<typename T1, typename T2>
		static constexpr u64 OffsetOf(T1 T2::* member)
		{
			T2 object{};
			return usize(&(object.*member)) - usize(&object);
		}

	public:
		TypeDispatcher() = delete;
		~TypeDispatcher() = delete;
	};
}