#pragma once

#include <Runtime/Containers/List.h>
#include <Runtime/Containers/String.h>

#include <Runtime/Reflection/TypeCodes.h>
#include <Runtime/Reflection/TypeModes.h>
#include <Runtime/Reflection/EnumValue.h>
#include <Runtime/Reflection/Attribute.h>

namespace Horizon
{
	class Field;

	using DefaultHeapObjectGenerator = void* (*)(void);

	class H_EXPORT Type final
	{
		friend class TypeDispatcher;
		friend struct Allocator;
	public:
		FORCEINLINE String GetName() const noexcept { return m_name; }
		FORCEINLINE String GetNamespaces() const noexcept { return m_namespaces; }
		FORCEINLINE String GetFullName() const noexcept { return m_fullName; }
		FORCEINLINE String GetRawName() const noexcept { return m_rawName; }
		FORCEINLINE u32 GetSize() const noexcept { return m_size; }
		FORCEINLINE TypeModes GetMode() const noexcept { return m_mode; }
		FORCEINLINE TypeCodes GetCode() const noexcept { return m_code; }
		FORCEINLINE u32 GetShareCount() const noexcept { return m_shareCount; }
		FORCEINLINE const List<Field*>& GetFields() const noexcept { return m_fields; }
		FORCEINLINE const List<Attribute*>& GetAttributes() const noexcept { return m_attributes; }
		FORCEINLINE const List<EnumValue>& GetEnums() const noexcept { return m_enums; }

		Attribute* GetCustomAttribute(const Type* pType) const noexcept;
		String GetEnumName(const i64 value) const noexcept;
		Field* GetField(const String& name) const noexcept;

		template<typename T>
		String GetEnumName(const T value) const noexcept
		{
			for (const EnumValue& enumValue : m_enums)
				if (enumValue.Value == (i64)value)
					return enumValue.Name;
			return "";
		}

		template<typename T>
		T GetEnumValue(const String& value) const noexcept
		{
			for (const EnumValue& enumValue : m_enums)
				if (enumValue.Name == value)
					return (T)enumValue.Value;
			return (T)0;
		}

		template<typename T>
		T* GetCustomAttribute() const noexcept
		{
			Type* pType = typeof(T);
			for (Attribute* pAttribute : m_attributes)
				if (pType == pAttribute->GetType())
					return (T*)pAttribute;
			return nullptr;
		}

		template<typename T>
		List<T*> GetCustomAttributes() const noexcept
		{
			List<T*> attributes;
			Type* pType = typeof(T);
			for (Attribute* pAttribute : m_attributes)
				if (pType == pAttribute->GetType())
					attributes.PushBack((T*)pAttribute);
			return attributes;
		}

		void* CreateDefaultHeapObject() const noexcept;
		b8 IsSubClassOf(const Type* pType) const noexcept;

	private:
		Type(const String& name, const String& namespaces, const u32 size, const TypeModes mode, const TypeCodes code, DefaultHeapObjectGenerator defaultObjectGenerator, Type** ppModuleAddress);
		~Type();

		void RegisterEnum(const String& name, const i64 value);
		void RegisterField(Field* pField);
		void SetBaseType(Type* pType);
		void RegisterAttribute(Attribute* pAttribute);
		void IncrementShareCount();
		void DecrementShareCount();

	private:
		const String m_name;
		const String m_namespaces;
		const String m_fullName;
		const String m_rawName;
		const u32 m_size;
		const TypeModes m_mode;
		const TypeCodes m_code;
		const DefaultHeapObjectGenerator m_defaultObjectGenerator;
		List<Field*> m_fields;
		List<Attribute*> m_attributes;
		List<EnumValue> m_enums;
		Type* m_baseType;
		Type** m_moduleAddress;
		u32 m_shareCount;
	};
}