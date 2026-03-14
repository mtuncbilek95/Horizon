#pragma once

namespace Horizon
{
	enum class ShaderStage
	{
		Vertex = 1 << 0,
		TessControl = 1 << 1,
		TessEval = 1 << 2,
		Geometry = 1 << 3,
		Fragment = 1 << 4,
		Compute = 1 << 5,
		Raygen = 1 << 6,
		Anyhit = 1 << 7,
		ClosestHit = 1 << 8,
		Miss = 1 << 9,
		Intersection = 1 << 10,
		Callable = 1 << 11,
		Task = 1 << 12,
		Mesh = 1 << 13,
		All = 0x3FFF
	};
}
