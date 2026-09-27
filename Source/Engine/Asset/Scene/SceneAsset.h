#pragma once

#include <Engine/Asset/AssetObject.h>
#include <Engine/Job/JobSystem.h>
#include <Engine/World/ECS/Scene.h>

namespace Horizon::Engine
{
	HCLASS();
	class H_EXPORT SceneAsset : public AssetObject
	{
		HORIZON_TYPE_REFLECT(SceneAsset);
		friend class SceneAssetStreamer;
	public:
		SceneAsset();
		~SceneAsset();

		void LoadAsync();
		void UnloadAsync();

		Scene* GetScene() const { return m_scene; }

	private:
		SubmitTicket m_ticket = InvalidSubmitTicket;

		Scene* m_scene = nullptr;
	};
}