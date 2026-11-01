#pragma once

#include <Runtime/RHI/Object/Object.h>
#include <Runtime/RHI/Pipeline/PipelineLayoutDesc.h>

namespace Horizon::RHI
{
	class RUNTIME_API PipelineLayout : public Object
	{
	public:
		const List<DescriptorTableLayout>& GetTables() const { return m_tables; }
		u32 GetPushConstantSize() const { return m_pushConstantSize; }
		b8 IsBindless() const { return m_bindless; }

	protected:
		List<DescriptorTableLayout> m_tables;
		u32 m_pushConstantSize = 0;
		b8 m_bindless = false;
	};
}