#pragma once

#include <Engine/Core/System.h>

#include <Editor/Domain/DomainNode.h>

#include <Runtime/Containers/Guid.h>

#include <chrono>
#include <filesystem>
#include <unordered_map>

namespace Horizon
{
	class H_EXPORT DomainSystem : public System
	{
	public:
		EngineReport OnAttach(Engine* pEngine) final;
		void OnSync() final;
		void OnDetach() final;

		DomainNode* GetRoot() const { return m_root; }
		DomainNode* FindById(const Guid& id) const;

	private:
		void Reconcile();

		DomainNode* BuildFolder(const std::filesystem::path& absPath, DomainNode* pParent);
		DomainNode* BuildAsset(const std::filesystem::path& sourcePath, DomainNode* pParent);

		Guid LoadOrCreateMeta(const std::filesystem::path& sourcePath, std::filesystem::path& outMetaPath);
		Guid ReadMetaId(const std::filesystem::path& metaPath) const;
		void WriteMetaId(const std::filesystem::path& metaPath, const Guid& id);

		void EnsureCooked(const Guid& id, const std::filesystem::path& sourcePath);
		void PruneCooked();

		void ClearTree(DomainNode* node);
		void Register(DomainNode* node);

		static std::filesystem::path MetaPathFor(const std::filesystem::path& sourcePath);
		static b8 IsMeta(const std::filesystem::path& path);
	
	private:
		std::filesystem::path m_assetsRoot;
		std::filesystem::path m_cookedRoot;

		DomainNode* m_root = nullptr;
		std::unordered_map<Guid, DomainNode*> m_registry;

		std::chrono::steady_clock::time_point m_lastScan{};
	};
}