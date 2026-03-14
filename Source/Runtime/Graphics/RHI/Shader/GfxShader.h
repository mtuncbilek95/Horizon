#pragma once

#include <Runtime/Graphics/RHI/Object/GfxObject.h>
#include <Runtime/Graphics/RHI/Shader/GfxShaderDesc.h>

namespace Horizon
{
	class GfxShader : public GfxObject
	{
	public:
		GfxShader(const GfxShaderDesc& desc, GfxDevice* pDevice);
		virtual ~GfxShader() override = default;

		virtual void* Shader() const = 0;

		ShaderStage Type() const { return m_desc.stage; }

	private:
		GfxShaderDesc m_desc;
	};
}
