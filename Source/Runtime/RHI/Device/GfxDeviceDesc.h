#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::RHI
{
	/**
	 * @brief Creation descriptor of a GfxDevice. Every
	 * flag only turns on validation tooling, so all of
	 * them are meant to stay off in shipping builds.
	 *
	 * @code
	 *   RHI::GfxDeviceDesc devDesc = {};
	 *   devDesc.enableDebugLayer = true;
	 *   devDesc.enableGpuValidation = true;
	 * @endcode
	 */
	struct GfxDeviceDesc
	{
		b8 enableDebugLayer = false;
		b8 enableGpuValidation = false;
		b8 synchronizedCommandValidation = false;
	};
}