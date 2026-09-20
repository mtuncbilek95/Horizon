#include "DomainFile.h"

#include <Editor/Domain/DomainFolder.h>
#include <Engine/Asset/AssetHeader.h>
#include <Runtime/Containers/StringOps.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/PAL/File/File.h>

#include <cstring>
#include <utility>

namespace Horizon::Editor
{
	DomainFile::DomainFile(DomainFolder* pParent, const std::string& name, const std::string& metaPath, const std::string& sourcePath, const std::string& cookFolder) :
		m_parent(pParent), m_name(name), m_metaPath(metaPath), m_sourcePath(sourcePath), m_cookFolder(cookFolder)
	{
	}

	DomainFile::~DomainFile()
	{
	}

	std::string DomainFile::GetRelativePath() const
	{
		const std::string parentPath = m_parent->GetRelativePath();

		if (parentPath.empty())
			return m_name;

		return parentPath + "/" + m_name;
	}

	std::string DomainFile::GetExtension() const
	{
		const usize dot = m_name.find_last_of('.');

		if (dot == std::string::npos)
			return std::string();

		std::string extension = m_name.substr(dot);

		for (c8& character : extension)
			character = StringOps::ToLowerAscii(character);

		return extension;
	}

	std::string DomainFile::GetCookedPath() const
	{
		if (!m_meta.id.IsValid())
		{
			Terminal::Error(StringOps::GetName(this), "{} has no id to resolve a cooked path", m_name);
			return std::string();
		}

		return m_cookFolder + "/" + m_meta.id.ToString() + std::string(CookSuffix);
	}

	b8 DomainFile::HasMeta() const
	{
		return PAL::File::Exists(m_metaPath);
	}

	b8 DomainFile::HasSource() const
	{
		return PAL::File::Exists(m_sourcePath);
	}

	b8 DomainFile::HasBinary() const
	{
		if (!m_meta.id.IsValid())
			return false;

		return PAL::File::Exists(GetCookedPath());
	}

	b8 DomainFile::LoadMetaFile()
	{
		if (!PAL::File::Exists(m_metaPath))
			return false;

		DomainMeta meta;

		if (!meta.Read(m_metaPath))
			return false;

		m_meta = std::move(meta);

		return true;
	}

	b8 DomainFile::WriteMetaFile(const DomainMeta& meta)
	{
		if (!meta.Write(m_metaPath))
			return false;

		m_meta = meta;

		return true;
	}

	b8 DomainFile::WriteCookFile(const List<u8>& content, usize propertySize)
	{
		const std::string cookedPath = GetCookedPath();

		if (cookedPath.empty())
			return false;

		if (m_meta.assetTypeName.empty() || m_meta.assetTypeName.size() >= MaxTypeBufferLength)
		{
			Terminal::Error(StringOps::GetName(this), "{} has an asset type name that cannot fit the header", m_name);
			return false;
		}

		if (propertySize > content.GetCount())
		{
			Terminal::Error(StringOps::GetName(this), "{} has a property size of {} but only {} bytes of content", m_name, propertySize, content.GetCount());
			return false;
		}

		Engine::AssetHeader header = {};
		header.magic = Engine::AssetHeader::Magic;
		header.version = Engine::AssetHeader::Version;
		header.id = m_meta.id;
		std::memcpy(header.typeName, m_meta.assetTypeName.data(), m_meta.assetTypeName.size());
		header.propertyOffset = sizeof(Engine::AssetHeader);
		header.propertySize = propertySize;
		header.payloadOffset = sizeof(Engine::AssetHeader) + propertySize;
		header.payloadSize = content.GetCount() - propertySize;

		List<u8> bytes(sizeof(Engine::AssetHeader) + content.GetCount());
		std::memcpy(bytes.GetData(), &header, sizeof(Engine::AssetHeader));

		if (!content.IsEmpty())
			std::memcpy(bytes.GetData() + sizeof(Engine::AssetHeader), content.GetData(), content.GetCount());

		if (PAL::File::Exists(cookedPath) && !PAL::File::Delete(cookedPath))
		{
			Terminal::Error(StringOps::GetName(this), "{} cannot be replaced", cookedPath);
			return false;
		}

		if (!PAL::File::Create(cookedPath))
		{
			Terminal::Error(StringOps::GetName(this), "{} cannot be created", cookedPath);
			return false;
		}

		PAL::FileAccessRequest request = PAL::File::RequestAccess(cookedPath, PAL::FileOperationAccessPolicy::Write,
			PAL::FileOperationSharePolicy::Exclusive);

		const b8 wasWritten = PAL::File::WriteMemory(request, bytes);

		PAL::File::ReleaseAccess(request);

		if (!wasWritten)
		{
			Terminal::Error(StringOps::GetName(this), "{} cannot be written", cookedPath);
			return false;
		}

		return true;
	}

	void DomainFile::Rename(const std::string& newName)
	{
		std::string newPath = m_parent->GetAbsolutePath() + "/" + newName;
		PAL::File::Rename(m_sourcePath, newPath);
	}
}