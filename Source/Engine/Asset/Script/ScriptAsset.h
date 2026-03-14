#pragma once

#include <Engine/Asset/Asset.h>
#include <Engine/Asset/Scene/SceneAsset.h>

namespace Horizon
{
	class GameManager;

	class ScriptAsset : public Asset<ScriptAsset>
	{
	public:
		virtual ~ScriptAsset() = default;

		virtual void OnStart() {}
		virtual void OnTick() {}
		virtual void OnPause() {}

		template<typename T>
		T& GetComponent()
		{
			return m_scene->GetComponent<T>(m_entity);
		}

		template<typename T>
		bool HasComponent() const
		{
			return m_scene->HasComponent<T>(m_entity);
		}

	protected:
		friend class GameManager;

		void Bind(SceneAsset* scene, entt::entity entity)
		{
			m_scene  = scene;
			m_entity = entity;
		}

	private:
		SceneAsset* m_scene = nullptr;
		entt::entity m_entity = entt::null;
	};
}
