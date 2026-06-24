#pragma once

#include <Runtime/Containers/String.h>
#include <Runtime/Containers/List.h>

#include <Runtime/Reflection/TypeModes.h>
#include <Runtime/Reflection/TypeCodes.h>
#include <Runtime/Reflection/FieldMode.h>
#include <Runtime/Reflection/EnumValue.h>
#include <Runtime/Reflection/Type.h>
#include <Runtime/Reflection/Attribute.h>

namespace Horizon
{
	struct H_EXPORT ReflectionFieldDescriptor final
	{
		String Name;
		i32 FieldTypeIndex;
		FieldMode Mode;
		u32 OffsetInBytes;
	};

	struct H_EXPORT ReflectionTypeDescriptor final
	{
		String Name;
		String Namespaces;
		u32 SizeInBytes;
		TypeModes Mode;
		TypeCodes Code;
		DefaultHeapObjectGenerator DefaultObjectGeneratorFunction;
		List<ReflectionFieldDescriptor> Fields;
		List<Attribute*> Attributes;
		List<EnumValue> Enums;
		i32 BaseTypeIndex;
		Type** ppLibraryAddress;
	};

	struct H_EXPORT ReflectionManifestDescriptor final
	{
		List<ReflectionTypeDescriptor> Types;
	};
}