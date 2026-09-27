#pragma once

#include <Editor/Domain/DomainFile.h>
#include <Editor/Renderer/EditorContext.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Editor
{
	class H_EXPORT AssetAction : public Reflect::Base
	{
	public:
		virtual void OnTrigger(EditorContext* pContext, DomainFile* pUsedAsset) = 0;
	};
}