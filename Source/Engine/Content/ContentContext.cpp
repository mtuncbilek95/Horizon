#include "ContentContext.h"

#include <Runtime/Containers/StringOps.h>
#include <Runtime/Definitions/Allocator.h>

namespace Horizon::Engine
{
	ContentContext::ContentContext(List<ContentMount*>&& mounts) : m_mounts(std::move(mounts))
	{
	}

	ModuleReport ContentContext::OnInitialize()
	{
		if (m_mounts.IsEmpty())
			Terminal::Warn(StringOps::GetName(this), "No content mount is registered, no asset can be resolved");

		for (usize i = m_mounts.GetCount(); i > 0; --i)
		{
			ContentMount* pMount = m_mounts[i - 1];

			if (pMount->Mount())
			{
				Terminal::Info(StringOps::GetName(this), "{} mount is ready", pMount->GetName());
				continue;
			}

			Terminal::Error(StringOps::GetName(this), "{} mount failed and is dropped", pMount->GetName());

			Memory::Allocator::Delete(pMount);
			m_mounts.RemoveAt(i - 1);
		}

		return ModuleReport();
	}

	void ContentContext::OnFinalize()
	{
		for (ContentMount* pMount : m_mounts)
		{
			pMount->Unmount();
			Memory::Allocator::Delete(pMount);
		}

		m_mounts.Clear();
	}

	void ContentContext::DeclareDependencies(ModuleGraph& graph)
	{
		(void)graph;
	}

	b8 ContentContext::Contains(const Guid& guid) const
	{
		for (ContentMount* pMount : m_mounts)
		{
			if (pMount->Contains(guid))
				return true;
		}

		return false;
	}

	b8 ContentContext::Read(const Guid& guid, List<u8>& outPayload) const
	{
		ContentMount* pMount = FindMount(guid);

		if (pMount == nullptr)
			return false;

		return pMount->Read(guid, outPayload);
	}

	b8 ContentContext::ReadRange(const Guid& guid, u64 offset, u64 size, List<u8>& outPayload) const
	{
		ContentMount* pMount = FindMount(guid);

		if (pMount == nullptr)
			return false;

		return pMount->ReadRange(guid, offset, size, outPayload);
	}

	void ContentContext::Enumerate(List<Guid>& outGuids) const
	{
		for (ContentMount* pMount : m_mounts)
			pMount->Enumerate(outGuids);
	}

	ContentMount* ContentContext::FindMutableMount() const
	{
		for (ContentMount* pMount : m_mounts)
		{
			if (pMount->IsMutable())
				return pMount;
		}

		Terminal::Error(StringOps::GetName(this), "No mutable mount is registered, nothing can be imported");
		return nullptr;
	}

	ContentMount* ContentContext::FindMount(const Guid& guid) const
	{
		for (ContentMount* pMount : m_mounts)
		{
			if (pMount->Contains(guid))
				return pMount;
		}

		Terminal::Error(StringOps::GetName(this), "{} is not carried by any mount", guid.ToString());
		return nullptr;
	}
}