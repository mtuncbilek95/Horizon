#pragma once

#include <Runtime/RHI/Shader/GfxShader.h>
#include <Runtime/D3D12/D3D12Helpers.h>

#include <Runtime/Containers/List.h>

namespace Horizon::RHI
{
	class D3D12Device;

	class D3D12Shader final : public GfxShader
	{
		friend class D3D12Device;
	public:
		~D3D12Shader() final = default;

		D3D12_SHADER_BYTECODE Bytecode() const { return { m_byteCode.GetData(), m_byteCode.GetCount() }; }
		b8 IsValid() const { return m_byteCode.GetCount() > 0; }

	private:
		List<u8> m_byteCode;
	};
}
