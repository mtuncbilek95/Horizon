#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/RHI/Descriptor/GfxDescriptorHeapType.h>

namespace Horizon::RHI
{
	/**
	 * @brief Creation descriptor of a GfxDescriptorHeap.
	 * The capacity is fixed at creation time, so it has to
	 * cover the worst case slot count of a frame.
	 *
	 * @code
	 *   RHI::GfxDescriptorHeapDesc heapDesc = {};
	 *   heapDesc.type = RHI::GfxDescriptorHeapType::Resource;
	 *   heapDesc.capacity = 100000;
	 *   heapDesc.shaderVisible = true;
	 * @endcode
	 */
	struct GfxDescriptorHeapDesc
	{
		GfxDescriptorHeapType type = GfxDescriptorHeapType::Resource;
		u32 capacity = 0;
		b8 shaderVisible = false;
	};
}