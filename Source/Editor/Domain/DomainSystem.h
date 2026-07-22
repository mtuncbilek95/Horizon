#pragma once

#include <Editor/Domain/DomainFolder.h>
#include <Editor/Domain/ImportPipeline/ImportDescriptor.h>
#include <Editor/Domain/ImportPipeline/ImportFileSettings.h>
#include <Engine/Core/System.h>
#include <Runtime/RTTR/Reflection.h>

#include <filesystem>
#include <unordered_map>

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

		void ImportDefault(DomainFolder* targetFolder, const ImportDescriptor& descriptor);

	private:
		void UpdateFolder(DomainFolder* pTarget);
		void UpdateFile(DomainFile* pTarget);

	private:
		DomainFolder* m_rootFolder = nullptr;
		std::unordered_map<Reflect::TypeHandle, ImportFileSettings*> m_importSettings;
	};
}