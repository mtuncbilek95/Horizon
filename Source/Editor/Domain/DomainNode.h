#pragma once

#include <Editor/Domain/DomainNodeType.h>

#include <Runtime/Containers/Guid.h>
#include <Runtime/Containers/List.h>

#include <filesystem>

namespace Horizon
{
	class H_EXPORT DomainNode
	{
		friend class DomainSystem;
	public:
		DomainNode() = default;
		~DomainNode() = default;

		const Guid& GetId() const { return m_id; }

		const std::string& GetName() const { return m_name; }
		const std::filesystem::path& GetMetaPath() const { return m_metaFile; }
		const std::filesystem::path& GetSourcePath() const { return m_sourceFile; }

		DomainNodeType GetType() const { return m_type; }
		b8 IsFolder() const { return m_type == DomainNodeType::Folder; }

		DomainNode* GetParent() const { return m_parent; }
		b8 IsRoot() const { return m_parent == nullptr; }

		const List<DomainNode*>& GetItemList() const { return m_items; }

	private:
		Guid m_id;

		std::string m_name;
		std::filesystem::path m_metaFile;
		std::filesystem::path m_sourceFile;

		DomainNodeType m_type = DomainNodeType::File;

		DomainNode* m_parent = nullptr;
		List<DomainNode*> m_items;
	};
}