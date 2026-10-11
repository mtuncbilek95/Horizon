#pragma once

#include <Engine/Asset/Script/ScriptField.h>
#include <Engine/Asset/Script/ScriptFieldOverride.h>
#include <Engine/World/ECS/Definitions.h>
#include <Engine/World/System.h>
#include <Engine/World/SystemOrderAttribute.h>
#include <Runtime/Containers/Guid.h>
#include <Runtime/Containers/List.h>
#include <Runtime/RTTR/Reflection.h>

#include <string_view>
#include <unordered_map>

namespace Horizon::Engine
{
	class AssetService;
	class ScriptAsset;
	class ScriptComponent;
	class ScriptContext;

	enum class ScriptInstanceState : u8
	{
		Running,
		Failed
	};

	struct ScriptInstance
	{
		static constexpr i32 InvalidRef = -2;

		EntityHandle entity;
		Guid scriptId;
		i32 selfRef = InvalidRef;
		ScriptInstanceState state = ScriptInstanceState::Running;
		u64 lastSeenFrame = 0;
	};

	HCLASS(SystemOrder[1000]);
	class ENGINE_API ScriptSystem : public System
	{
		HORIZON_TYPE_REFLECT(ScriptSystem);
	public:
		b8 OnInitialize() final;
		void OnExecute(const EngineFrame& ctx, Scene& currentScene) final;
		void OnFinalize() final;
		void OnGroupsChanged(SystemGroup previous, SystemGroup current) final;

		SystemGroup GetWorkingGroup() const final { return SystemGroup::Script; }

	private:
		void BeginSession(Scene* pScene);
		void EndSession(Scene* pScene);

		ScriptAsset* ResolveAsset(const ScriptComponent& comp);
		ScriptInstance* FindInstance(EntityHandle entity);
		ScriptInstance* CreateInstance(EntityHandle entity, ScriptComponent& comp, const ScriptAsset& asset);
		i32 LoadClass(const ScriptAsset& asset);
		b8 IsAssetOfType(const ScriptField& field, const Guid& id) const;
		void UpdateInstance(ScriptInstance& instance, ScriptComponent& comp, const ScriptAsset& asset, f32 deltaTime);
		void StopInstance(ScriptInstance& instance);
		void SweepInstances();

		static const ScriptField* FindField(const List<ScriptField>& fields, std::string_view name);
		static const ScriptFieldOverride* FindOverride(const ScriptComponent& comp, std::string_view name);
		static const ScriptFieldValue& ResolveValue(const ScriptComponent& comp, const ScriptField& field);
		static EntityHandle FindEntityRef(const ScriptComponent& comp, std::string_view name);

	private:
		WorldService* m_worldService = nullptr;
		ScriptContext* m_context = nullptr;
		AssetService* m_assetService = nullptr;

		std::unordered_map<u32, ScriptInstance> m_instances;
		std::unordered_map<Guid, i32> m_classRefs;
		u64 m_frame = 0;
		b8 m_inSession = false;
	};
}