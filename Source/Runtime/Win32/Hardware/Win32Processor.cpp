#include <Runtime/PAL/Hardware/Processor.h>

#include <Runtime/Log/Terminal.h>

#include <Windows.h>
#include <algorithm>

namespace Horizon::PAL
{
	List<CoreInfo> Processor::EnumerateCores()
	{
		List<CoreInfo> result;

		DWORD len = 0;
		GetLogicalProcessorInformationEx(RelationProcessorCore, nullptr, &len);

		if (len == 0)
		{
			Terminal::Error("Processor", "GetLogicalProcessorInformationEx reported no core relation data");
			return result;
		}

		List<u8> buffer(len);
		if (!GetLogicalProcessorInformationEx(RelationProcessorCore,
			(SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*)(buffer.GetData()), &len))
		{
			Terminal::Error("Processor", "GetLogicalProcessorInformationEx failed with {}", ::GetLastError());
			return result;
		}

		auto forEachCore = [&](auto&& fn)
			{
				DWORD offset = 0;
				while (offset < len)
				{
					auto* rec = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*)(buffer.GetData() + offset);

					if (rec->Relationship == RelationProcessorCore)
						fn(*rec);

					offset += rec->Size;
				}
			};

		BYTE maxClass = 0;
		forEachCore([&](const SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX& rec)
			{
				maxClass = (std::max)(maxClass, rec.Processor.EfficiencyClass);
			});

		u32 physicalIndex = 0;
		forEachCore([&](const SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX& rec)
			{
				if (rec.Processor.GroupCount == 0)
					return;

				const KAFFINITY mask = rec.Processor.GroupMask[0].Mask;
				const u8 efficiencyClass = (u8)(rec.Processor.EfficiencyClass);
				const b8 isPerf = (rec.Processor.EfficiencyClass == maxClass);

				b8 primary = true;

				for (u32 bit = 0; bit < sizeof(KAFFINITY) * 8; ++bit)
				{
					if (!(mask & (KAFFINITY(1) << bit)))
						continue;

					result.PushBack(CoreInfo{ bit, physicalIndex, efficiencyClass, isPerf, primary });
					primary = false;
				}

				++physicalIndex;
			});

		return result;
	}

	u32 Processor::PerformanceCoreCount()
	{
		u32 count = 0;

		for (const CoreInfo& core : EnumerateCores())
		{
			if (core.isPerformance)
				++count;
		}

		return count;
	}

	u32 Processor::EfficiencyCoreCount()
	{
		u32 count = 0;

		for (const CoreInfo& core : EnumerateCores())
		{
			if (!core.isPerformance)
				++count;
		}

		return count;
	}

	u32 Processor::PhysicalCoreCount()
	{
		u32 count = 0;

		for (const CoreInfo& core : EnumerateCores())
		{
			if (core.isPrimarySibling)
				++count;
		}

		return count;
	}
}