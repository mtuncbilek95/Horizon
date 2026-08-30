#pragma once

#include <Engine/Content/ContentMount.h>
#include <Engine/Core/Context.h>
#include <Runtime/Containers/List.h>

namespace Horizon::Engine
{
	class H_EXPORT ContentContext final : public Context
	{
	public:
		ContentContext(List<ContentMount*>&& mounts);
		~ContentContext() = default;

		ModuleReport OnInitialize() final;
		void OnFinalize() final;
		void DeclareDependencies(ModuleGraph& graph) final;

		b8 Contains(const Guid& guid) const;
		b8 Read(const Guid& guid, List<u8>& outPayload) const;
		b8 ReadRange(const Guid& guid, u64 offset, u64 size, List<u8>& outPayload) const;
		void Enumerate(List<Guid>& outGuids) const;

		ContentMount* FindMutableMount() const;

	private:
		ContentMount* FindMount(const Guid& guid) const;

	private:
		List<ContentMount*> m_mounts;
	};
}