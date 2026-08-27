#pragma once

#include <Runtime/RTTR/Attribute.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Reflect
{
	class H_EXPORT BlobAttribute final : public Attribute
	{
		HORIZON_ATTRIBUTE_REFLECT(BlobAttribute);
	public:
		BlobAttribute() = default;
		~BlobAttribute() = default;
	};
}