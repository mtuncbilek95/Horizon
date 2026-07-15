#pragma once

#include <Editor/Domain/DomainFolder.h>
#include <Engine/Core/System.h>
#include <Runtime/Reflection/TypeManifest.h>

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

		void AddNewFolder(DomainFolder* targetFolder);
		void ImportDefault(DomainFolder* targetFolder, const std::string& fileTypeExt);

	private:
		void UpdateFolder(DomainFolder* pTarget);

	private:
		std::filesystem::path m_rootPath;
		DomainFolder* m_rootFolder = nullptr;

		std::vector<TypeManifest*> m_importerManifest;
	};
}