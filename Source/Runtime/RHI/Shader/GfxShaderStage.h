#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::RHI
{
	/**
	 * @brief Bitwise pipeline stage flags. A single shader
	 * carries exactly one of them, while the masks are used
	 * where a whole set of stages is addressed.
	 *
	 * @code
	 *   RHI::GfxShaderDesc shaderDesc = {};
	 *   shaderDesc.stage = RHI::GfxShaderStage::Mesh;
	 * @endcode
	 */
	enum class GfxShaderStage : u32
	{
		None = 0,

		Vertex = 1 << 0,
		TessCtrl = 1 << 1,
		TessEval = 1 << 2,
		Geometry = 1 << 3,
		Pixel = 1 << 4,

		Compute = 1 << 5,

		Task = 1 << 6,
		Mesh = 1 << 7,

		RayGeneration = 1 << 8,
		Miss = 1 << 9,
		ClosestHit = 1 << 10,
		AnyHit = 1 << 11,
		Intersection = 1 << 12,
		Callable = 1 << 13
	};
}