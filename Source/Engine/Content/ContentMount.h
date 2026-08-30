#pragma once

#include <Runtime/Containers/Guid.h>
#include <Runtime/Containers/List.h>
#include <Runtime/Definitions/PrimitiveDefinitions.h>

#include <string>

namespace Horizon::Engine
{
	class H_EXPORT ContentMount
	{
	public:
		ContentMount(const std::string& name) : m_name(name)
		{
		}

		virtual ~ContentMount() = default;

		ContentMount(const ContentMount&) = delete;
		ContentMount& operator=(const ContentMount&) = delete;

		virtual b8 Mount() = 0;
		virtual void Unmount() = 0;

		virtual b8 Contains(const Guid& guid) const = 0;
		virtual b8 Read(const Guid& guid, List<u8>& outPayload) const = 0;
		virtual b8 ReadRange(const Guid& guid, u64 offset, u64 size, List<u8>& outPayload) const = 0;
		virtual void Enumerate(List<Guid>& outGuids) const = 0;

		virtual b8 IsMutable() const { return false; }

		const std::string& GetName() const { return m_name; }

	private:
		std::string m_name;
	};
}