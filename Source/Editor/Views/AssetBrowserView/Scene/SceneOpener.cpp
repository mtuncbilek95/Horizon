#include "SceneOpener.h"

#include <Editor/Domain/DomainFile.h>

#include <Engine/Core/Engine.h>
#include <Engine/World/WorldService.h>

#include <Runtime/Log/Terminal.h>

namespace Horizon::Editor
{
	b8 SceneOpener::Open(Engine::Engine* pEngine, DomainFile* pFile)
	{
		Engine::WorldService* pWorld = pEngine->RequestService<Engine::WorldService>();

		if (pWorld == nullptr)
			return false;

		return pWorld->LoadScene(pFile->GetID());
	}
}