#pragma once

#include <Editor/Domain/DomainFolder.h>
#include <Editor/Domain/ImportDescriptor.h>
#include <Engine/Core/System.h>
#include <Runtime/RTTR/Reflection.h>

#include <filesystem>

namespace Horizon
{
	class IAssetImporter;

	class H_EXPORT DomainSystem : public System
	{
	public:
		DomainSystem() = default;
		~DomainSystem() = default;

		EngineReport OnAttach(Engine* pEngine);
		void OnSync();
		void OnDetach();

		void GetInitializeOrder(OrderRules& rules) const final;
		void GetExecutionOrder(OrderRules& rules) const final;

		DomainFolder* GetRootFolder() const { return m_rootFolder; }

		void ImportDefault(DomainFolder* targetFolder, const ImportDescriptor& importInfo);

	private:
		void UpdateFolder(DomainFolder* pTarget);

	private:
		std::filesystem::path m_rootPath;
		DomainFolder* m_rootFolder = nullptr;

		std::vector<Reflect::Type*> m_importerManifest;
	};
}