#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

#include <Runtime/RHI/Shader/GfxShaderStage.h>

namespace Horizon::RHI
{
	/**
	 * @brief Creation descriptor of a GfxShader. The byte
	 * code is only borrowed for the duration of the call,
	 * so the caller stays the owner of the blob.
	 *
	 * @code
	 *   RHI::GfxShaderDesc shaderDesc = {};
	 *   shaderDesc.stage = RHI::GfxShaderStage::Pixel;
	 *   shaderDesc.pByteCode = blob.Data();
	 *   shaderDesc.byteCodeSize = blob.Size();
	 * @endcode
	 */
	struct GfxShaderDesc
	{
		GfxShaderStage stage = GfxShaderStage::None;

		const void* pByteCode = nullptr;
		usize byteCodeSize = 0;

		const char* pEntryPoint = "main";
	};
}
