#include "TypeDispatcher.h"

namespace Horizon
{
	Type* TypeDispatcher::CreateType(const String& name, const String& namespaces, const u32 sizeInBytes,
		const TypeModes mode, const TypeCodes code, const DefaultHeapObjectGenerator generator, Type** ppAddress)
	{
		return Allocator::Create<Type>(CurrLoc(), name, namespaces, sizeInBytes, mode, code, generator, ppAddress);
	}

	void TypeDispatcher::DeleteType(Type* pType)
	{
		if (pType == nullptr)
			return;

		for (Field* pField : pType->m_fields)
			Allocator::Delete(pField);

		for (Attribute* pAttribute : pType->m_attributes)
			Allocator::Delete(pAttribute);

		Allocator::Delete(pType);
	}

	void TypeDispatcher::DeleteField(Field* pField)
	{
		Allocator::Delete(pField);
	}

	void TypeDispatcher::RegisterEnum(const String& name, const i64 value, Type* pTargetType)
	{
		pTargetType->RegisterEnum(name, value);
	}

	void TypeDispatcher::RegisterField(const String& name, const u32 offset, Type* pFieldType,
		const FieldMode mode, Type* pTargetType)
	{
		Field* pField = Allocator::Create<Field>(CurrLoc(), name, pFieldType, mode, offset);
		pTargetType->RegisterField(pField);
	}

	void TypeDispatcher::RegisterAttribute(Type* pTargetType, Attribute* pAttribute)
	{
		pTargetType->RegisterAttribute(pAttribute);
	}

	void TypeDispatcher::SetBaseType(Type* pTargetType, Type* pBaseType)
	{
		pTargetType->SetBaseType(pBaseType);
	}

	void TypeDispatcher::IncrementTypeShare(Type* pTargetType)
	{
		pTargetType->IncrementShareCount();
	}

	void TypeDispatcher::DecrementTypeShare(Type* pTargetType)
	{
		pTargetType->DecrementShareCount();
	}

	Type** TypeDispatcher::GetTypeAddress(const Type* pType)
	{
		return pType->m_moduleAddress;
	}
}