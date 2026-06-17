#pragma once

namespace Horizon
{
	class Type;

	template<typename T>
	class TypeAccessor {};

	template<typename T>
	inline Type* typeof() { return TypeAccessor<T>::GetType(); }

	typedef void* (*DefaultHeapObjectGenerator)(void);

#define HCLASS(...)
#define HENUM(...)
#define HFIELD(...)
#define HATTRIBUTE(target,...)
}