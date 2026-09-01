#include "DomainMeta.h"

#include <Runtime/Containers/StringOps.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/PAL/File/File.h>
#include <Runtime/Serialization/JsonArchive.h>

#include <utility>

namespace Horizon::Editor
{
	b8 DomainMeta::Read(const std::string& metaPath)
	{
		PAL::FileAccessRequest request = PAL::File::RequestAccess(metaPath, PAL::FileOperationAccessPolicy::Read,
			PAL::FileOperationSharePolicy::SharedRead);

		std::string text;
		const b8 wasRead = PAL::File::ReadString(request, text);

		PAL::File::ReleaseAccess(request);

		if (!wasRead)
		{
			Terminal::Error(StringOps::GetName(this), "{} cannot be read", metaPath);
			return false;
		}

		JsonArchiveReader reader(text);

		if (reader.HasError())
		{
			Terminal::Error(StringOps::GetName(this), "{} is malformed", metaPath);
			return false;
		}

		reader.BeginObject();

		if (reader.Key("version") && reader.ReadU64() > Version)
		{
			Terminal::Error(StringOps::GetName(this), "{} is newer than the supported version {}", metaPath, Version);
			return false;
		}

		if (reader.Key("id"))
			id = Guid(reader.ReadString());

		if (reader.Key("assetType"))
			assetTypeName = reader.ReadString();

		subAssets.Clear();

		if (reader.Key("subAssets"))
		{
			const usize count = reader.BeginArray();

			for (usize i = 0; i < count; ++i)
			{
				reader.BeginObject();

				DomainSubAsset subAsset;

				if (reader.Key("name"))
					subAsset.name = reader.ReadString();

				if (reader.Key("assetType"))
					subAsset.assetTypeName = reader.ReadString();

				if (reader.Key("id"))
					subAsset.id = Guid(reader.ReadString());

				reader.EndObject();

				subAssets.PushBack(std::move(subAsset));
			}

			reader.EndArray();
		}

		reader.EndObject();

		if (!id.IsValid())
		{
			Terminal::Error(StringOps::GetName(this), "{} carries no valid id", metaPath);
			return false;
		}

		return true;
	}

	b8 DomainMeta::Write(const std::string& metaPath) const
	{
		JsonArchiveWriter writer;

		writer.BeginObject();

		writer.Key("version");
		writer.WriteU64(Version);

		writer.Key("id");
		writer.WriteString(id.ToString());

		writer.Key("assetType");
		writer.WriteString(assetTypeName);

		writer.Key("subAssets");
		writer.BeginArray(subAssets.GetCount());

		for (const DomainSubAsset& subAsset : subAssets)
		{
			writer.BeginObject();

			writer.Key("name");
			writer.WriteString(subAsset.name);

			writer.Key("assetType");
			writer.WriteString(subAsset.assetTypeName);

			writer.Key("id");
			writer.WriteString(subAsset.id.ToString());

			writer.EndObject();
		}

		writer.EndArray();
		writer.EndObject();

		if (!PAL::File::Create(metaPath))
		{
			Terminal::Error(StringOps::GetName(this), "{} cannot be created", metaPath);
			return false;
		}

		PAL::FileAccessRequest request = PAL::File::RequestAccess(metaPath, PAL::FileOperationAccessPolicy::Write,
			PAL::FileOperationSharePolicy::Exclusive);

		const b8 wasWritten = PAL::File::WriteString(request, writer.ToString());

		PAL::File::ReleaseAccess(request);

		if (!wasWritten)
		{
			Terminal::Error(StringOps::GetName(this), "{} cannot be written", metaPath);
			return false;
		}

		return true;
	}
}