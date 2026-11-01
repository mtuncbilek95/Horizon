#pragma once

#include <Runtime/RHI/Object/Object.h>
#include <Runtime/RHI/Shader/ShaderModuleDesc.h>

namespace Horizon::RHI
{
	class RUNTIME_API ShaderModule : public Object
	{
	public:
		ShaderStage GetStage() const { return m_stage; }
		const std::string& GetEntryPoint() const { return m_entryPoint; }
		const List<u8>& GetByteCode() const { return m_byteCode; }

	protected:
		ShaderStage m_stage = ShaderStage::None;
		std::string m_entryPoint;
		List<u8> m_byteCode;
	};
}