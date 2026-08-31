#pragma once

#include <Editor/Views/AssetBrowserView/AssetOpenerAttribute.h>
#include <Editor/Views/AssetBrowserView/AssetOpener.h>
#include <Engine/Asset/Scene/SceneAsset.h>

namespace Horizon::Editor
{
	HCLASS(AssetOpener[Reflect::TypeOf<Engine::SceneAsset>()]);
	class H_EXPORT SceneOpener final : public AssetOpener
	{
		HORIZON_TYPE_REFLECT(SceneOpener);
	public:
		b8 Open(Engine::Engine* pEngine, DomainFile* pFile) final;
	};
}