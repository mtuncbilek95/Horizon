#include "DomainSystem.h"

#include <Runtime/PAL/File/File.h>

#include <Runtime/Definitions/Allocator.h>
#include <Runtime/Log/Terminal.h>

namespace Horizon
{
	EngineReport DomainSystem::OnAttach(Engine* pEngine)
	{
		const std::filesystem::path projectRoot = R"(D:\Projects\Horizon\ExampleProject)";

		m_assetsRoot = projectRoot / "Assets";
		m_cookedRoot = projectRoot / "Cooked";

		return EngineReport();
	}

	void DomainSystem::OnSync()
	{
		const auto now = std::chrono::steady_clock::now();

		if (now - m_lastScan < std::chrono::seconds(1))
			return;

		m_lastScan = now;
		Reconcile();
	}

	void DomainSystem::OnDetach()
	{
		if (m_root)
			ClearTree(m_root);

		m_root = nullptr;
		m_registry.clear();
	}

	DomainNode* DomainSystem::FindById(const Guid& id) const
	{
		auto it = m_registry.find(id);

		if (it == m_registry.end())
		{
			Terminal::Warn("DomainSystem", "{} has no node", id.ToString());
			return nullptr;
		}

		return it->second;
	}

	void DomainSystem::Reconcile()
	{
		if (!std::filesystem::exists(m_assetsRoot))
		{
			Terminal::Error("DomainSystem", "Assets root missing: {}", m_assetsRoot.string());
			return;
		}

		std::filesystem::create_directories(m_cookedRoot);

		if (m_root)
			ClearTree(m_root);

		m_registry.clear();

		m_root = BuildFolder(m_assetsRoot, nullptr);

		PruneCooked();
	}

	DomainNode* DomainSystem::BuildFolder(const std::filesystem::path& absPath, DomainNode* parent)
	{
		auto* node = Allocator::Create<DomainNode>(CurrLoc());
		node->m_type = DomainNodeType::Folder;
		node->m_name = absPath.filename().string();
		node->m_sourceFile = absPath;
		node->m_parent = parent;

		List<std::filesystem::path> subDirs;
		List<std::filesystem::path> sources;
		List<std::filesystem::path> metas;

		for (const auto& entry : std::filesystem::directory_iterator(absPath))
		{
			if (entry.is_directory())
				subDirs.PushBack(entry.path());
			else if (IsMeta(entry.path()))
				metas.PushBack(entry.path());
			else
				sources.PushBack(entry.path());
		}

		for (const auto& meta : metas)
		{
			const std::filesystem::path source = meta.parent_path() / meta.stem();

			if (!std::filesystem::exists(source))
			{
				const Guid id = ReadMetaId(meta);

				PAL::File::Delete(meta);
				PAL::File::Delete(m_cookedRoot / id.ToString());

				Terminal::Warn("DomainSystem", "removed orphan meta {}", meta.filename().string());
			}
		}

		for (const auto& source : sources)
			node->m_items.PushBack(BuildAsset(source, node));

		for (const auto& dir : subDirs)
			node->m_items.PushBack(BuildFolder(dir, node));

		return node;
	}

	DomainNode* DomainSystem::BuildAsset(const std::filesystem::path& sourcePath, DomainNode* parent)
	{
		std::filesystem::path metaPath;
		const Guid id = LoadOrCreateMeta(sourcePath, metaPath);

		auto* node = Allocator::Create<DomainNode>(CurrLoc());
		node->m_type = DomainNodeType::File;
		node->m_name = sourcePath.filename().string();
		node->m_sourceFile = sourcePath;
		node->m_metaFile = metaPath;
		node->m_id = id;
		node->m_parent = parent;

		EnsureCooked(id, sourcePath);
		Register(node);

		return node;
	}

	Guid DomainSystem::LoadOrCreateMeta(const std::filesystem::path& sourcePath, std::filesystem::path& outMetaPath)
	{
		outMetaPath = MetaPathFor(sourcePath);

		if (std::filesystem::exists(outMetaPath))
			return ReadMetaId(outMetaPath);

		const Guid id = Guid::Generate();
		WriteMetaId(outMetaPath, id);

		Terminal::Info("DomainSystem", "created meta for {}", sourcePath.filename().string());
		return id;
	}

	Guid DomainSystem::ReadMetaId(const std::filesystem::path& metaPath) const
	{
		PAL::FileAccessRequest req = PAL::File::RequestAccess(metaPath,
			PAL::FileOperationAccessPolicy::Read, PAL::FileOperationSharePolicy::SharedRead);

		if (!req.IsValid())
		{
			Terminal::Error("DomainSystem", "cannot open meta for read: {}", metaPath.string());
			return Guid();
		}

		List<u8> bytes;
		PAL::File::ReadMemory(req, bytes);
		PAL::File::ReleaseAccess(req);

		if (bytes.IsEmpty())
		{
			Terminal::Error("DomainSystem", "empty meta: {}", metaPath.string());
			return Guid();
		}

		const std::string text(reinterpret_cast<const char*>(bytes.GetData()), bytes.GetCount());
		return Guid(text);
	}

	void DomainSystem::WriteMetaId(const std::filesystem::path& metaPath, const Guid& id)
	{
		PAL::File::Create(metaPath);

		PAL::FileAccessRequest req = PAL::File::RequestAccess(metaPath,
			PAL::FileOperationAccessPolicy::Write, PAL::FileOperationSharePolicy::Exclusive);

		if (!req.IsValid())
		{
			Terminal::Error("DomainSystem", "cannot open meta for write: {}", metaPath.string());
			return;
		}

		PAL::File::WriteString(req, id.ToString());
		PAL::File::ReleaseAccess(req);
	}

	void DomainSystem::EnsureCooked(const Guid& id, const std::filesystem::path& sourcePath)
	{
		const std::filesystem::path cookedPath = m_cookedRoot / id.ToString();

		if (std::filesystem::exists(cookedPath))
			return;

		PAL::FileAccessRequest readReq = PAL::File::RequestAccess(sourcePath,
			PAL::FileOperationAccessPolicy::Read, PAL::FileOperationSharePolicy::SharedRead);

		if (!readReq.IsValid())
		{
			Terminal::Error("DomainSystem", "cannot read source: {}", sourcePath.string());
			return;
		}

		// TODO: This Read and write is not a proper way.
		// AssetImportSettings and DomainImportSettings is not the same thing.

		List<u8> bytes;
		PAL::File::ReadMemory(readReq, bytes);
		PAL::File::ReleaseAccess(readReq);

		PAL::File::Create(cookedPath);

		PAL::FileAccessRequest writeReq = PAL::File::RequestAccess(cookedPath,
			PAL::FileOperationAccessPolicy::Write, PAL::FileOperationSharePolicy::Exclusive);

		if (!writeReq.IsValid())
		{
			Terminal::Error("DomainSystem", "cannot write cooked: {}", cookedPath.string());
			return;
		}

		PAL::File::WriteMemory(writeReq, bytes);
		PAL::File::ReleaseAccess(writeReq);

		Terminal::Info("DomainSystem", "cooked {} -> {}", sourcePath.filename().string(), id.ToString());
	}

	void DomainSystem::PruneCooked()
	{
		for (const auto& entry : std::filesystem::directory_iterator(m_cookedRoot))
		{
			const Guid id(entry.path().filename().string());

			if (m_registry.find(id) == m_registry.end())
			{
				PAL::File::Delete(entry.path());
				Terminal::Warn("DomainSystem", "removed orphan cooked {}", entry.path().filename().string());
			}
		}
	}

	void DomainSystem::ClearTree(DomainNode* node)
	{
		if (!node)
			return;

		for (auto* child : node->m_items)
			ClearTree(child);

		Allocator::Delete(node);
	}

	void DomainSystem::Register(DomainNode* node)
	{
		m_registry[node->m_id] = node;
	}

	std::filesystem::path DomainSystem::MetaPathFor(const std::filesystem::path& sourcePath)
	{
		std::filesystem::path meta = sourcePath;
		meta += ".hmeta";

		return meta;
	}

	b8 DomainSystem::IsMeta(const std::filesystem::path& path)
	{
		return path.extension() == ".hmeta";
	}
}