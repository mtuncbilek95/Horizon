#include <Runtime/PAL/Hardware/Processor.h>

#include <Windows.h>

namespace Horizon::PAL
{
	List<CoreInfo> Processor::EnumerateCores()
	{
		List<CoreInfo> result;

		DWORD len = 0;
		::GetLogicalProcessorInformationEx(RelationProcessorCore, nullptr, &len);
		if (len == 0)
			return result;

		List<u8> buffer(len);
		if (!::GetLogicalProcessorInformationEx(RelationProcessorCore,
			(SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*)(buffer.GetData()), &len))
			return result;

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

		forEachCore([&](const SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX& rec)
			{
				const b8 isPerf = (rec.Processor.EfficiencyClass == maxClass);

				if (rec.Processor.GroupCount == 0)
					return;

				const KAFFINITY mask = rec.Processor.GroupMask[0].Mask;
				for (u32 bit = 0; bit < sizeof(KAFFINITY) * 8; ++bit)
				{
					if (mask & (KAFFINITY(1) << bit))
					{
						result.PushBack(CoreInfo{ bit, isPerf });
						break;
					}
				}
			});

		return result;
	}

	u32 Processor::PerformanceCoreCount()
	{
		u32 count = 0;
		for (const CoreInfo& c : EnumerateCores())
			if (c.isPerformance)
				++count;

		return count;
	}

	u32 Processor::EfficiencyCoreCount()
	{
		u32 count = 0;
		for (const CoreInfo& c : EnumerateCores())
			if (!c.isPerformance)
				++count;

		return count;
	}
}