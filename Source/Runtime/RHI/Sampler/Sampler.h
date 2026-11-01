#pragma once

#include <Runtime/RHI/Object/Object.h>
#include <Runtime/RHI/Sampler/SamplerDesc.h>

namespace Horizon::RHI
{
	class RUNTIME_API Sampler : public Object
	{
	public:
		const SamplerDesc& GetDesc() const { return m_desc; }

	protected:
		SamplerDesc m_desc;
	};
}
