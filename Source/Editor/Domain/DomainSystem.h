#pragma once

#include <Engine/Core/System.h>
#include <Editor/Domain/DomainFolder.h>

#include <filesystem>

namespace Horizon
{
	class H_EXPORT DomainSystem : public System
	{
	public:
		DomainSystem() = default;
		~DomainSystem() = default;

		SystemReport OnAttach(Engine* pEngine);
		void OnSync();
		void OnDetach();

		void GetExecutionOrder(OrderRules& rules) const final;

	private:
		void RecursiveDebugChecker(DomainFolder* folder);
		void UpdateFolder(DomainFolder* pTarget);

	private:
		std::filesystem::path m_rootPath;
		DomainFolder* m_rootFolder;
	};
}