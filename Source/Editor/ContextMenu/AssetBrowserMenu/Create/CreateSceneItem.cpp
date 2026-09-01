#include "CreateSceneItem.h"

#include <Runtime/Containers/StringOps.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/PAL/File/File.h>
#include <Runtime/Serialization/JsonArchive.h>

#include <format>

namespace Horizon::Editor
{
	void CreateSceneItem::OnExecute(AssetBrowserContext& context)
	{
	}

	b8 CreateSceneItem::IsEnabled(const AssetBrowserContext& context)
	{
		return true;
	}
}