#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

#include <Runtime/RHI/Object/GfxObject.h>
#include <Runtime/RHI/Shader/GfxShaderStage.h>

namespace Horizon::RHI
{
	class GfxShader : public GfxObject
	{
	public:
		GfxShaderStage GetStage() const { return m_stage; }
		usize GetByteCodeSize() const { return m_byteCodeSize; }
	protected:
		GfxShaderStage m_stage = GfxShaderStage::None;
		usize m_byteCodeSize = 0;
	};
}
