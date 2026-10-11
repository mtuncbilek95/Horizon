#include "ScriptAssetAction.h"

#include <Runtime/PAL/Explorer/Command.h>

namespace Horizon::Editor
{
	void ScriptAssetAction::OnTrigger(EditorContext* pContext, DomainFile* pUsedAsset)
	{
		std::string usedAssetPath = pUsedAsset->GetSourcePath();
		PAL::Command::ExecuteInTerminal("code -r \"" + usedAssetPath + "\"");
	}
}