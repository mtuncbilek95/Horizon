#pragma once

#include <Runtime/Reflection/Type.h>
#include <Runtime/Reflection/FieldMode.h>

namespace Horizon
{
	class TypeDispatcher final
	{
	public:
		static Type* CreateType(const std::string& name, const std::string& namespaces, u32 sizeInBytes, TypeModes mode, TypeCodes code, 
			DefaultHeapObjectGenerator generator, Type** ppAddress);
		static void DeleteType(Type* pType);
		static void DeleteField(Field* pField);
		static void RegisterEnum(const std::string& name, i64 value, Type* pTargetType);
		static void RegisterField(const std::string& name, u32 offset, Type* pFieldType, FieldMode mode, Type* pTargetType);
		static void RegisterAttribute(Type* pTargetType, Attribute* pAttribute);
		static void SetBaseType(Type* pTargetType, Type* pBaseType);
		static void IncrementTypeShare(Type* pTargetType);
		static void DecrementTypeShare(Type* pTargetType);
		static Type** GetTypeAddress(Type* pType);

		template<typename T>
		static Type** GetTypeAddress() { return TypeAccessor<T>::GetTypeAddress(); }

		template<typename T>
		static void SetTypeAddress(Type* pType) { TypeAccessor<T>::SetType(pType); }

		template<typename T, typename... TArgs>
		static void RegisterAttribute(Type* pTargetType, TArgs... args) { pTargetType->RegisterAttribute(new T(args...)); }

		template <typename T1, typename T2>
		static inline u64 constexpr OffsetOf(T1 T2::* member)
		{
			T2 object{};
			return size_t(&(object.*member)) - size_t(&object);
		}

	public:
		TypeDispatcher() = delete;
		~TypeDispatcher() = delete;
	};
}