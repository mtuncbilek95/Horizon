#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::RHI
{
	/**
	 * @brief What happens to the stored stencil value
	 * when a test succeeds or fails. The saturating
	 * variants clamp while the wrapping ones roll over.
	 *
	 * @code
	 *   RHI::GfxStencilFace face = {};
	 *   face.failOp = RHI::GfxStencilOp::Keep;
	 *   face.passOp = RHI::GfxStencilOp::IncrSat;
	 * @endcode
	 */
	enum class GfxStencilOp : u8
	{
		Keep, 
		Zero,
		Replace,
		IncrSat, 
		DecrSat, 
		Invert,
		IncrWrap, 
		DecrWrap
	};
}