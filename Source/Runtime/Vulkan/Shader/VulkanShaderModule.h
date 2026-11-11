#pragma once

#include <Runtime/Containers/Vector/ReadOnlyList.h>
#include <Runtime/RHI/Shader/ShaderModule.h>

namespace Horizon::RHI
{
	class VulkanShaderModule final : public ShaderModule
	{
		friend class VulkanDevice;
	public:
		ReadOnlyList<const u8> Bytecode() const { return { m_byteCode.GetData(), m_byteCode.GetCount() }; }
	};
}