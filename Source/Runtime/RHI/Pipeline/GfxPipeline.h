#pragma once

#include <Runtime/RHI/Object/GfxObject.h>
#include <Runtime/RHI/Pipeline/GfxPipelineType.h>

namespace Horizon::RHI
{
	class GfxPipeline : public GfxObject
	{
	public:
		GfxPipelineType GetType() const { return m_type; }
	protected:
		GfxPipelineType m_type = GfxPipelineType::Graphics;
	};
}
