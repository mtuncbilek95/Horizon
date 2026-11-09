#pragma once

#include <Engine/Asset/AssetKind.h>
#include <Engine/Asset/AssetTypeAttribute.h>
#include <Engine/Asset/AssetObject.h>
#include <Engine/Asset/AssetResidency.h>
#include <Engine/Job/JobSystem.h>
#include <Engine/World/ECS/Scene.h>
#include <Runtime/Containers/Guid.h>
#include <Runtime/Containers/Memory/UniquePtr.h>
#include <Runtime/PAL/Sync/Atomic.h>

#include <string>

namespace Horizon::Asset
{
	HCLASS(Engine::AssetType[Engine::AssetKind::Streamable]);
	class Scene : public Engine::AssetObject
	{
		HORIZON_TYPE_REFLECT(Scene);
		friend class SceneStreamer;
	public:
		Scene();
		~Scene();

		void LoadAsync();
		void UnloadAsync();

		void LoadSync();
		void UnloadSync();

		const Guid& GetId() const { return m_id; }
		Engine::AssetResidency GetResidency() const { return m_residency.Load(PAL::MemoryOrder::Acquire); }
		Engine::Scene* GetScene() const { return m_scene.Get(); }

	protected:
		void OnPropertiesChanged(ReadOnlyList<u8> propertyBytes) {}

	private:
		Guid m_id;
		std::string m_cookPath;
		PAL::Atomic<Engine::AssetResidency> m_residency = Engine::AssetResidency::Unloaded;
		Engine::SubmitTicket m_ticket = Engine::InvalidSubmitTicket;
		UniquePtr<Engine::Scene> m_scene = nullptr;
	};
}