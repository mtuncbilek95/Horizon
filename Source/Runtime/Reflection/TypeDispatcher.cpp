#include "TypeDispatcher.h"

#include <Runtime/Reflection/Field.h>

namespace Horizon
{
	Type* TypeDispatcher::CreateType(const std::string& name, const std::string& namespaces, u32 size, TypeModes mode, TypeCodes code, 
		DefaultHeapObjectGenerator generator, Type** ppAddress)
	{
		return new Type(name, namespaces, size, mode, code, generator, ppAddress);
	}

	void TypeDispatcher::DeleteType(Type* pType) { delete pType; }
	void TypeDispatcher::DeleteField(Field* pField) { delete pField; }
	void TypeDispatcher::RegisterEnum(const std::string& name, i64 value, Type* pTargetType) { pTargetType->RegisterEnum(name, value); }

	void TypeDispatcher::RegisterField(const std::string& name, u32 offset, Type* pFieldType, FieldMode mode, Type* pTargetType)
	{
		Field* pField = new Field(name, pFieldType, mode, offset);
		pTargetType->RegisterField(pField);
	}

	void TypeDispatcher::RegisterAttribute(Type* pTargetType, Attribute* pAttribute) { pTargetType->RegisterAttribute(pAttribute); }
	void TypeDispatcher::SetBaseType(Type* pTargetType, Type* pBaseType) { pTargetType->SetBaseType(pBaseType); }
	void TypeDispatcher::IncrementTypeShare(Type* pTargetType) { pTargetType->IncrementShareCount(); }
	void TypeDispatcher::DecrementTypeShare(Type* pTargetType) { pTargetType->DecrementShareCount(); }
	Type** TypeDispatcher::GetTypeAddress(Type* pType) { return pType->m_moduleAddress; }
}