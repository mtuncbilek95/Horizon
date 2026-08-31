#include "SaveSceneMenuItem.h"

#include <Editor/Domain/DomainService.h>
#include <Editor/Domain/DomainFile.h>

#include <Engine/Asset/Scene/SceneInstantiator.h>
#include <Engine/Core/Engine.h>
#include <Engine/Reflection/ReflectionSystem.h>
#include <Engine/World/WorldService.h>

#include <Runtime/Log/Terminal.h>
#include <Runtime/PAL/File/File.h>
#include <Runtime/Serialization/JsonArchive.h>

namespace Horizon::Editor
{
	void SaveSceneMenuItem::OnExecute()
	{
		Engine::WorldService* pWorldService = GetEngine()->RequestService<Engine::WorldService>();
		DomainService* pDomain = GetEngine()->RequestService<DomainService>();

		if (pWorldService == nullptr || pDomain == nullptr)
			return;

		const Guid& sceneId = pWorldService->GetActiveSceneId();

		if (!sceneId.IsValid())
		{
			Terminal::Warn("SaveSceneMenuItem", "No scene is open, there is nothing to save");
			return;
		}

		Engine::World* pWorld = pWorldService->GetActiveWorld();

		if (pWorld == nullptr)
			return;

		DomainFile* pFile = pDomain->FindFileByGuid(sceneId);

		if (pFile == nullptr)
			return;

		JsonArchiveWriter writer;

		if (!Engine::SceneInstantiator::Capture(*pWorld, GetEngine()->GetReflectionSystem(), writer))
		{
			Terminal::Error("SaveSceneMenuItem", "{} could not be captured", pFile->GetName());
			return;
		}

		const std::string& scenePath = pFile->GetSourcePath();

		if (!PAL::File::Create(scenePath))
		{
			Terminal::Error("SaveSceneMenuItem", "{} cannot be created", scenePath);
			return;
		}

		PAL::FileAccessRequest request = PAL::File::RequestAccess(scenePath, PAL::FileOperationAccessPolicy::Write,
			PAL::FileOperationSharePolicy::Exclusive);

		const b8 wasWritten = PAL::File::WriteString(request, writer.ToString());

		PAL::File::ReleaseAccess(request);

		if (!wasWritten)
		{
			Terminal::Error("SaveSceneMenuItem", "{} cannot be written", scenePath);
			return;
		}

		Terminal::Info("SaveSceneMenuItem", "{} has been saved", scenePath);
	}
}