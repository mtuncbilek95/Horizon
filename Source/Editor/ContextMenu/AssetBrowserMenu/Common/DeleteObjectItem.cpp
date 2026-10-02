#include "DeleteObjectItem.h"

#include <Runtime/PAL/File/Directory.h>
#include <Runtime/PAL/File/File.h>

namespace Horizon::Editor
{
	void DeleteObjectItem::OnExecute(AssetBrowserContext& context)
	{
		for (auto* pFile : context.selectedFiles)
		{
			// TODO: This can work for now
			PAL::File::Delete(pFile->GetSourcePath());
			PAL::File::Delete(pFile->GetMetaPath());
		}

		for (auto* pFolder : context.selectedFolders)
			PAL::Directory::Delete(pFolder->GetAbsolutePath());
	}

	b8 DeleteObjectItem::IsEnabled(const AssetBrowserContext& context)
	{
		return context.selectedFiles.GetCount() != 0 || context.selectedFolders.GetCount() != 0;
	}
}