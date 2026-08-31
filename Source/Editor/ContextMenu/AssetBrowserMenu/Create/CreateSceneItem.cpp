#include "CreateSceneItem.h"

#include <Engine/Asset/Scene/SceneInstantiator.h>

#include <Runtime/Containers/StringOps.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/PAL/File/File.h>
#include <Runtime/Serialization/JsonArchive.h>

#include <format>

namespace Horizon::Editor
{
	namespace
	{
		constexpr std::string_view SceneSuffix = ".hscene";
	}

	void CreateSceneItem::OnExecute(AssetBrowserContext& context)
	{
		std::string sceneName = std::format("EmptyScene{}", SceneSuffix);

		if (context.currentFolder->FindFile(sceneName) != nullptr)
		{
			i32 index = 1;

			do
			{
				sceneName = std::format("EmptyScene({}){}", index, SceneSuffix);
				index++;
			} while (context.currentFolder->FindFile(sceneName) != nullptr);
		}

		const std::string scenePath = context.currentFolder->GetAbsolutePath() + "/" + sceneName;

		JsonArchiveWriter writer;
		Engine::SceneInstantiator::CaptureEmpty(writer);

		if (!PAL::File::Create(scenePath))
		{
			Terminal::Error("CreateSceneItem", "{} cannot be created", scenePath);
			return;
		}

		PAL::FileAccessRequest request = PAL::File::RequestAccess(scenePath, PAL::FileOperationAccessPolicy::Write,
			PAL::FileOperationSharePolicy::Exclusive);

		const b8 wasWritten = PAL::File::WriteString(request, writer.ToString());

		PAL::File::ReleaseAccess(request);

		if (!wasWritten)
			Terminal::Error("CreateSceneItem", "{} cannot be written", scenePath);
	}

	b8 CreateSceneItem::IsEnabled(const AssetBrowserContext& context)
	{
		return context.selectedFiles.GetCount() == 0 && context.selectedFolders.GetCount() == 0;
	}
}