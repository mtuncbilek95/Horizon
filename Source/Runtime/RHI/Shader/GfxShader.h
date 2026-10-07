#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

#include <Runtime/RHI/Object/GfxObject.h>
#include <Runtime/RHI/Shader/GfxShaderStage.h>

namespace Horizon::RHI
{
	class RUNTIME_API GfxShader : public GfxObject
	{
	public:
		GfxShaderStage GetStage() const { return m_stage; }
		usize GetByteCodeSize() const { return m_byteCodeSize; }
		const c8* GetDebugName() const { return m_debugName; }

		void SetDebugName(const char* pName) final;

	protected:
		GfxShaderStage m_stage = GfxShaderStage::None;
		usize m_byteCodeSize = 0;
		c8 m_debugName[MaxTypeBufferLength] = {};
	};
}
