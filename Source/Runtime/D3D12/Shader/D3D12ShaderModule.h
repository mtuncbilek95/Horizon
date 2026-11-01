#pragma once

#include <Runtime/RHI/Shader/ShaderModule.h>

#include <dxgi1_6.h>
#include <d3d12.h>

namespace Horizon::RHI
{
	class D3D12ShaderModule final : public ShaderModule
	{
		friend class D3D12Device;
	public:
		D3D12_SHADER_BYTECODE Bytecode() const { return { m_byteCode.GetData(), m_byteCode.GetCount() }; }
	};
}