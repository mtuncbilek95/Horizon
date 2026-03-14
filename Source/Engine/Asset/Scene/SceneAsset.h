#pragma once

#include <Engine/Asset/Asset.h>

#include <entt/entt.hpp>

namespace Horizon
{
	class SceneAsset : public Asset<SceneAsset>
	{
	public:
		SceneAsset() = default;
		virtual ~SceneAsset() = default;

		bool Load() final;
		void Unload() final;
		bool IsLoaded() const final { return m_loaded; }

		entt::entity CreateEntity() { return m_sceneRegistry.create(); }
		void DestroyEntity(entt::entity entity) { m_sceneRegistry.destroy(entity); }

		template<typename T, typename... Args>
		T& AddComponent(entt::entity entity, Args&&... args) { return m_sceneRegistry.emplace<T>(entity, std::forward<Args>(args)...); }

		template<typename T>
		T& GetComponent(entt::entity entity) { return m_sceneRegistry.get<T>(entity); }

		template<typename T>
		bool HasComponent(entt::entity entity) const { return m_sceneRegistry.all_of<T>(entity); }

		entt::registry& GetRegistry() { return m_sceneRegistry; }

	private:
		entt::registry m_sceneRegistry;
		bool m_loaded = false;
	};
}