#pragma once

namespace Horizon
{
	class Type;

	template<typename T>
	class TypeAccessor
	{
	};

#define typeof(type) TypeAccessor<type>::GetType()

}

#define HCLASS(...)
#define HFIELD(...)
#define HATTRIBUTE(...)
#define HENUM(...)
#define H_GENERATE_REFLECTION

#define GENERATE_OBJECT