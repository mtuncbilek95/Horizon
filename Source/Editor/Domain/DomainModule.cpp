#include "DomainModule.h"

#include <Engine/Engine/Engine.h>

namespace Horizon
{
	void DomainModule::OnAttach(Engine& engine)
	{
		IModule::OnAttach(engine);
	}

	void DomainModule::OnSync()
	{}

	void DomainModule::OnDetach()
	{}
}