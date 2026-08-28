#pragma once

#include <Runtime/RHI/Object/GfxObject.h>
#include <Runtime/RHI/Pipeline/GfxPipelineType.h>

namespace Horizon::RHI
{
	/**
	 * @brief Compiled pipeline state object. Graphics and
	 * compute pipelines share this type and are told apart
	 * by GetType.
	 *
	 * @code
	 *   myCmdList->BindPipeline(myPipeline);
	 * @endcode
	 */
	class GfxPipeline : public GfxObject
	{
	public:
		GfxPipelineType GetType() const { return m_type; }

	protected:
		GfxPipelineType m_type = GfxPipelineType::Graphics;
	};
}
