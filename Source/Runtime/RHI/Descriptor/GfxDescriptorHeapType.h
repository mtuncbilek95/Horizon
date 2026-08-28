#pragma once

#include <Runtime/Definitions/BitwiseOperators.h>
#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::RHI
{
	/**
	 * @brief Kind of descriptors a heap is able to store.
	 * Resource and Sampler heaps are shader visible, while
	 * Color and Depth heaps only feed the output merger.
	 *
	 * @code
	 *   RHI::GfxDescriptorHeapDesc heapDesc = {};
	 *   heapDesc.type = RHI::GfxDescriptorHeapType::Sampler;
	 * @endcode
	 */
	enum class GfxDescriptorHeapType : u8
	{
		Resource,
		Sampler,
		Color,
		Depth
	};
}