#pragma once

#include <Runtime/RHI/Object/Object.h>
#include <Runtime/RHI/Pipeline/PipelineType.h>

namespace Horizon::RHI
{
	class PipelineLayout;

	class Pipeline : public Object
	{
	public:
		PipelineType GetType() const { return m_type; }
		PipelineLayout* GetLayout() const { return m_layout; }

	protected:
		PipelineType m_type = PipelineType::Graphics;
		PipelineLayout* m_layout = nullptr;
	};
}
