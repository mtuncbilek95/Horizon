#include "LooseContentMount.h"

#include <Runtime/Containers/StringOps.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/PAL/File/Directory.h>
#include <Runtime/PAL/File/File.h>

namespace Horizon::Engine
{
	namespace
	{
		constexpr usize GuidStringLength = 36;
	}

	b8 LooseContentMount::Mount()
	{
		if (PAL::Directory::Exists(m_rootPath))
			return true;

		if (PAL::Directory::Create(m_rootPath))
			return true;

		Terminal::Error(StringOps::GetName(this), "{} content root cannot be created", m_rootPath);
		return false;
	}

	void LooseContentMount::Unmount()
	{
	}

	b8 LooseContentMount::Contains(const Guid& guid) const
	{
		return PAL::File::Exists(ToPath(guid));
	}

	b8 LooseContentMount::Read(const Guid& guid, List<u8>& outPayload) const
	{
		const std::string path = ToPath(guid);

		PAL::FileAccessRequest request = PAL::File::RequestAccess(path, PAL::FileOperationAccessPolicy::Read,
			PAL::FileOperationSharePolicy::SharedRead);

		const b8 wasRead = PAL::File::ReadMemory(request, outPayload);

		PAL::File::ReleaseAccess(request);

		if (!wasRead)
		{
			Terminal::Error(StringOps::GetName(this), "{} cannot be read", path);
			return false;
		}

		return true;
	}

	b8 LooseContentMount::ReadRange(const Guid& guid, u64 offset, u64 size, List<u8>& outPayload) const
	{
		const std::string path = ToPath(guid);

		PAL::FileAccessRequest request = PAL::File::RequestAccess(path, PAL::FileOperationAccessPolicy::Read,
			PAL::FileOperationSharePolicy::SharedRead);

		const b8 wasRead = PAL::File::ReadMemory(request, outPayload, (usize)offset, (usize)(offset + size));

		PAL::File::ReleaseAccess(request);

		if (!wasRead)
		{
			Terminal::Error(StringOps::GetName(this), "{} cannot be read at {} for {} bytes", path, offset, size);
			return false;
		}

		return true;
	}

	void LooseContentMount::Enumerate(List<Guid>& outGuids) const
	{
		const List<PAL::Directory::Entry> entries = PAL::Directory::Iterate(m_rootPath);

		for (const PAL::Directory::Entry& entry : entries)
		{
			if (entry.isDirectory || !entry.name.ends_with(FileSuffix))
				continue;

			const std::string stem = entry.name.substr(0, entry.name.size() - FileSuffix.size());

			if (stem.size() != GuidStringLength)
			{
				Terminal::Warn(StringOps::GetName(this), "{} is not named after a guid", entry.name);
				continue;
			}

			outGuids.PushBack(Guid(stem));
		}
	}

	std::string LooseContentMount::ToPath(const Guid& guid) const
	{
		return m_rootPath + "/" + guid.ToString() + std::string(FileSuffix);
	}
}