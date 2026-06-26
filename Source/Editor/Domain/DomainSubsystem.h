#pragma once

#include <Engine/Core/Subsystem.h>
#include <Editor/Domain/DomainFolder.h>

#include <filesystem>

namespace Horizon
{
	class H_EXPORT DomainSubsystem : public Subsystem
	{
	public:
		DomainSubsystem() = default;
		~DomainSubsystem() = default;

		EngineReport OnAttach(Engine* pEngine);
		void OnSync();
		void OnDetach();

		void GetExecutionOrder(OrderRules& rules) const final;

	private:
		std::filesystem::path m_rootPath;
		DomainFolder* m_rootFolder;
	};
}