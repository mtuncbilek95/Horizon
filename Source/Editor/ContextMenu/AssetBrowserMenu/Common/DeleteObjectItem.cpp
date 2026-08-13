#include "DeleteObjectItem.h"

#include <Runtime/PAL/File/Directory.h>
#include <Runtime/PAL/File/File.h>

namespace Horizon::Editor
{
	void DeleteObjectItem::OnExecute(AssetBrowserContext& context)
	{
		for (auto* file : context.selectedFiles)
		{
			// This can work for now
			PAL::File::Delete(file->GetSourcePath());
			PAL::File::Delete(file->GetMetaPath());
		}

		for (auto* folder : context.selectedFolders)
			PAL::Directory::Delete(folder->GetAbsolutePath());
	}

	b8 DeleteObjectItem::IsEnabled(const AssetBrowserContext& context)
	{
		return context.selectedFiles.GetCount() != 0 || context.selectedFolders.GetCount() != 0;
	}
}