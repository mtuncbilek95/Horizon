#include "RenameAssetItem.h"

#include <Editor/Domain/DomainNode.h>

namespace Horizon
{
	void RenameAssetItem::OnExecute(AssetBrowserMenuContext& context)
	{
		if (context.selected.IsEmpty())
			return;

		*context.renameRequest = context.selected.Front()->GetSourcePath();
	}

	b8 RenameAssetItem::IsEnabled(const AssetBrowserMenuContext& context) const
	{
		return context.selected.GetCount() == 1;
	}
}