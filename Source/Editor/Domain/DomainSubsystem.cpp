#include "DomainSubsystem.h"

#include <Engine/Core/Engine.h>
#include <Engine/Asset/AssetSubsystem.h>
#include <Engine/Command/CommandSubsystem.h>

#include <Editor/Domain/DomainFile.h>

namespace Horizon
{
	EngineReport DomainSubsystem::OnAttach(Engine* pEngine)
	{
		Subsystem::OnAttach(pEngine);

		auto* pCommandSub = m_engine->TryGetSubsystem<CommandSubsystem>();
		const auto& startPoint = pCommandSub->GetProjectPath();

		m_rootPath = startPoint / "Project";
		if (!std::filesystem::exists(m_rootPath))
			std::filesystem::create_directory(m_rootPath);

		DomainFolderDesc rootDesc = {};
		rootDesc.folderPath = m_rootPath;
		rootDesc.pParent = nullptr;
		m_rootFolder = Allocator::Create<DomainFolder>(CurrLoc(), rootDesc, m_engine);

#if defined(HORIZON_DEBUG)
		for (auto* file : m_rootFolder->GetFiles())
			Terminal::Log(m_rootFolder->GetName(), "{}", file->GetName());

		for (auto* folder : m_rootFolder->GetSubfolders())
		{
			Terminal::Log(m_rootFolder->GetName(), "{}", folder->GetName());
			RecursiveDebugChecker(folder);
		}
#endif

		return EngineReport();
	}

	void DomainSubsystem::OnSync()
	{
	}

	void DomainSubsystem::OnDetach()
	{
		Allocator::Delete(m_rootFolder);
	}

	void DomainSubsystem::GetExecutionOrder(OrderRules& rules) const
	{
		Requires<AssetSubsystem>(rules.after);
	}

	void DomainSubsystem::RecursiveDebugChecker(DomainFolder* folder)
	{
		for (auto* file : folder->GetFiles())
			Terminal::Log(folder->GetName(), "{}", file->GetName());

		for (auto* fd : folder->GetSubfolders())
			RecursiveDebugChecker(fd);
	}
}