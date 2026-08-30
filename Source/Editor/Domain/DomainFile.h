#pragma once

#include <Editor/Domain/DomainMeta.h>
#include <Runtime/Containers/Guid.h>
#include <string>
#include <string_view>

namespace Horizon::Editor
{
	class DomainFolder;

	class H_EXPORT DomainFile
	{
	public:
		static constexpr std::string_view MetaSuffix = ".hmeta";
		static constexpr std::string_view MetaExtension = "hmeta";

		DomainFile(DomainFolder* pParent, const std::string& name, const std::string& metaPath, const std::string& sourcePath) :
			m_parent(pParent), m_name(name), m_metaPath(metaPath), m_sourcePath(sourcePath)
		{
		}
		~DomainFile()
		{
		}

		const Guid& GetID() const { return m_meta.id; }
		const DomainMeta& GetMeta() const { return m_meta; }
		std::string GetRelativePath() const;
		DomainFolder* GetParent() const { return m_parent; }

		const std::string& GetName() const { return m_name; }

		const std::string& GetMetaPath() const { return m_metaPath; }
		const std::string& GetSourcePath() const { return m_sourcePath; }

		b8 EnsureMeta();
		b8 ReloadMeta();
		b8 SetAssetType(const std::string& assetTypeName);

		void Rename(const std::string& newName);

	private:
		DomainFolder* m_parent = nullptr;
		DomainMeta m_meta;

		std::string m_name;

		std::string m_metaPath;
		std::string m_sourcePath;
	};
}