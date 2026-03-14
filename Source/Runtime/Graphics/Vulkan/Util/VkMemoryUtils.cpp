#include "VkMemoryUtils.h"

#include <magic_enum/magic_enum.hpp>

namespace Horizon
{
    VmaMemoryUsage VmaUtils::GetVmaUsage(MemoryUsage usage)
    {
        VmaMemoryUsage flags = VMA_MEMORY_USAGE_UNKNOWN;

        if(HasFlag(usage, MemoryUsage::GpuLazy))
			flags |= VMA_MEMORY_USAGE_GPU_ONLY;
		else if(HasFlag(usage, MemoryUsage::Auto))
			flags |= VMA_MEMORY_USAGE_AUTO;
		else if(HasFlag(usage, MemoryUsage::AutoPreferDevice))
			flags |= VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
		else if(HasFlag(usage, MemoryUsage::AutoPreferHost))
			flags |= VMA_MEMORY_USAGE_AUTO_PREFER_HOST;

        return flags;
    }

    VmaAllocationCreateFlags VmaUtils::GetVmaFlags(MemoryAllocation alloc)
    {
        VmaAllocationCreateFlags flags = 0;

		if (HasFlag(alloc, MemoryAllocation::DedicatedMemory))
			flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
		if (HasFlag(alloc, MemoryAllocation::NeverAllocate))
			flags |= VMA_ALLOCATION_CREATE_NEVER_ALLOCATE_BIT;
		if (HasFlag(alloc, MemoryAllocation::Mapped))
			flags |= VMA_ALLOCATION_CREATE_MAPPED_BIT;
		if (HasFlag(alloc, MemoryAllocation::UpperAddress))
			flags |= VMA_ALLOCATION_CREATE_UPPER_ADDRESS_BIT;
		if (HasFlag(alloc, MemoryAllocation::DontBind))
			flags |= VMA_ALLOCATION_CREATE_DONT_BIND_BIT;
		if (HasFlag(alloc, MemoryAllocation::WithinBudget))
			flags |= VMA_ALLOCATION_CREATE_WITHIN_BUDGET_BIT;
		if (HasFlag(alloc, MemoryAllocation::CanAlias))
			flags |= VMA_ALLOCATION_CREATE_CAN_ALIAS_BIT;
		if (HasFlag(alloc, MemoryAllocation::SequentialWrite))
			flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
		if (HasFlag(alloc, MemoryAllocation::HostAccessRandom))
			flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
		if (HasFlag(alloc, MemoryAllocation::AllowTransferInstead))
			flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT;
		if (HasFlag(alloc, MemoryAllocation::StrategyMinMemory))
			flags |= VMA_ALLOCATION_CREATE_STRATEGY_MIN_MEMORY_BIT;
		if (HasFlag(alloc, MemoryAllocation::StrategyMinTime))
			flags |= VMA_ALLOCATION_CREATE_STRATEGY_MIN_TIME_BIT;
		if (HasFlag(alloc, MemoryAllocation::StrategyMinOffset))
			flags |= VMA_ALLOCATION_CREATE_STRATEGY_MIN_OFFSET_BIT;

		return flags;
    }

}
