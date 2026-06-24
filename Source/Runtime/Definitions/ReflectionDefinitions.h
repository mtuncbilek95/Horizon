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