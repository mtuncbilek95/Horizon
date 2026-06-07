#pragma once

#include <d3d12.h>

namespace Horizon
{
	struct DX12Pipeline
	{
		ID3D12PipelineState* pso = nullptr;
		b8 isCompute = false;
		D3D_PRIMITIVE_TOPOLOGY topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	};
}
