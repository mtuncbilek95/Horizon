#include "DomainService.h"

#include <Editor/Domain/DomainFolder.h>
#include <Editor/Domain/DomainFile.h>

#include <Engine/Core/ModuleGraph.h>

namespace Horizon::Editor
{
	DomainService::DomainService()
	{
	}

	DomainService::~DomainService()
	{
	}

	Engine::ModuleReport DomainService::OnInitialize()
	{
		return Engine::ModuleReport();
	}

	void DomainService::OnExecute()
	{
	}

	void DomainService::OnFinalize()
	{

	}

	void DomainService::DeclareDependencies(Engine::ModuleGraph& graph)
	{
	}
}