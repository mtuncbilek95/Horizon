#pragma once

#include <Engine/Content/ContentMount.h>

#include <string_view>

namespace Horizon::Engine
{
	class H_EXPORT LooseContentMount final : public ContentMount
	{
	public:
		static constexpr std::string_view FileSuffix = ".hfile";

		LooseContentMount(const std::string& name, const std::string& rootPath) : ContentMount(name),
			m_rootPath(rootPath)
		{
		}

		b8 Mount() final;
		void Unmount() final;

		b8 Contains(const Guid& guid) const final;
		b8 Read(const Guid& guid, List<u8>& outPayload) const final;
		b8 ReadRange(const Guid& guid, u64 offset, u64 size, List<u8>& outPayload) const final;
		b8 Write(const Guid& guid, const List<u8>& payload) final;

		b8 IsMutable() const final { return true; }

		void Enumerate(List<Guid>& outGuids) const;

		const std::string& GetRootPath() const { return m_rootPath; }

	private:
		std::string ToPath(const Guid& guid) const;

		std::string m_rootPath;
	};
}