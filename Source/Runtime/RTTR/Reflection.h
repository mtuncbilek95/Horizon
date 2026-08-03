#pragma once

#include <Runtime/RTTR/Attribute.h>
#include <Runtime/RTTR/EnumValue.h>
#include <Runtime/RTTR/Field.h>
#include <Runtime/RTTR/Type.h>
#include <Runtime/RTTR/TypeBuilder.h>
#include <Runtime/RTTR/TypeHandle.h>
#include <Runtime/RTTR/TypeKind.h>
#include <Runtime/RTTR/TypeMode.h>
#include <Runtime/RTTR/TypeResolve.h>

namespace Horizon::Reflect
{
	template<typename T>
	struct TypeAccessor;
}

#define HORIZON_TYPE_REFLECT							\
	template<typename>									\
	friend struct ::Horizon::Reflect::TypeAccessor

#define HCLASS(...)
#define HATTRIBUTE(...)
#define HFIELD(...)
#define HENUM(...)