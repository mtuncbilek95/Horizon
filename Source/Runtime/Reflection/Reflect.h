#pragma once

namespace Horizon
{
	template<typename T>
	struct Reflector;
}

#define HORIZON_REFLECT \
template<typename> \
friend struct ::Horizon::Reflector

#define HCLASS(...)
#define HFIELD(...)