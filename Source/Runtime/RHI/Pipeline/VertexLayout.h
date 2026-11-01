#pragma once

#include <Runtime/Containers/Vector/List.h>
#include <Runtime/RHI/Pipeline/VertexAttribute.h>
#include <Runtime/RHI/Pipeline/VertexBinding.h>

namespace Horizon::RHI
{
	struct VertexLayout final
	{
		List<VertexBinding> bindings;
		List<VertexAttribute> attributes;
	};
}
