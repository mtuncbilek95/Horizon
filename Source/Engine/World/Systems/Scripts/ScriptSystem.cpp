#include "ScriptSystem.h"

#include <Engine/Asset/AssetService.h>
#include <Engine/Asset/Script/ScriptAsset.h>
#include <Engine/Core/Engine.h>
#include <Engine/Script/ScriptContext.h>
#include <Engine/World/WorldService.h>
#include <Engine/World/Components/Scripts/ScriptComponent.h>

#include <Runtime/Containers/StringOps.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/Math/Quat.h>
#include <Runtime/Math/Vec2f.h>
#include <Runtime/Math/Vec3f.h>

#include <sol/sol.hpp>

#include <algorithm>
#include <string>

namespace Horizon::Engine
{
	namespace
	{
		sol::object ToLua(sol::state_view& lua, const ScriptField& field, const ScriptFieldValue& value, EntityHandle entity)
		{
			switch (field.kind)
			{
			case ScriptFieldKind::Bool:
				return sol::make_object(lua, value.boolValue != 0);
			case ScriptFieldKind::Integer:
				return sol::make_object(lua, value.integerValue);
			case ScriptFieldKind::Number:
				return sol::make_object(lua, value.numberValue);
			case ScriptFieldKind::String:
				return sol::make_object(lua, value.stringValue);
			case ScriptFieldKind::Vector2:
				return sol::make_object(lua, value.vec2Value);
			case ScriptFieldKind::Vector3:
				return sol::make_object(lua, value.vec3Value);
			case ScriptFieldKind::Quaternion:
				return sol::make_object(lua, value.quatValue);
			case ScriptFieldKind::Entity:
				return sol::make_object(lua, entity);
			case ScriptFieldKind::Asset:
				if (!value.assetValue.IsValid())
					return sol::make_object(lua, sol::lua_nil);

				return sol::make_object(lua, value.assetValue);
			default:
				return sol::make_object(lua, sol::lua_nil);
			}
		}

		void FromLua(const sol::object& object, ScriptLiveValue& outLive)
		{
			const sol::type type = object.get_type();

			switch (outLive.kind)
			{
			case ScriptFieldKind::Bool:
				if (type == sol::type::boolean)
					outLive.value.boolValue = object.as<bool>();
				break;
			case ScriptFieldKind::Integer:
				if (type == sol::type::number)
					outLive.value.integerValue = object.as<i64>();
				break;
			case ScriptFieldKind::Number:
				if (type == sol::type::number)
					outLive.value.numberValue = object.as<f64>();
				break;
			case ScriptFieldKind::String:
				if (type == sol::type::string)
					outLive.value.stringValue = object.as<std::string>();
				break;
			case ScriptFieldKind::Vector2:
				if (object.is<Math::Vec2f>())
					outLive.value.vec2Value = object.as<Math::Vec2f>();
				break;
			case ScriptFieldKind::Vector3:
				if (object.is<Math::Vec3f>())
					outLive.value.vec3Value = object.as<Math::Vec3f>();
				break;
			case ScriptFieldKind::Quaternion:
				if (object.is<Math::Quat>())
					outLive.value.quatValue = object.as<Math::Quat>();
				break;
			case ScriptFieldKind::Entity:
				if (object.is<EntityHandle>())
					outLive.entity = object.as<EntityHandle>();
				else if (type == sol::type::lua_nil)
					outLive.entity = EntityHandle();
				break;
			case ScriptFieldKind::Asset:
				if (object.is<Guid>())
					outLive.value.assetValue = object.as<Guid>();
				else if (type == sol::type::lua_nil)
					outLive.value.assetValue = Guid();
				break;
			}
		}

		template<typename... Args>
		b8 CallHook(const sol::table& self, const c8* pName, std::string& outError, Args&&... args)
		{
			const sol::object hook = self.get<sol::object>(pName);

			if (hook.get_type() != sol::type::function)
				return true;

			sol::protected_function function = hook.as<sol::protected_function>();
			sol::protected_function_result result = function(self, std::forward<Args>(args)...);

			if (result.valid())
				return true;

			sol::error error = result;
			outError = error.what();
			return false;
		}
	}

	b8 ScriptSystem::OnInitialize()
	{
		m_context = GetEngine()->RequestContext<ScriptContext>();
		m_assetService = GetEngine()->RequestService<AssetService>();

		return m_context != nullptr && m_assetService != nullptr;
	}

	void ScriptSystem::OnExecute(const EngineFrame& ctx, Scene& currentScene)
	{
		if (!m_worldService)
			m_worldService = GetEngine()->RequestService<WorldService>();

		// Just in case
		if (!HasFlag(m_worldService->GetRunningSystems(), SystemGroup::Script))
			return;

		if (!m_inSession)
			return;

		m_frame++;

		currentScene.ForEach<ScriptComponent>([&](EntityHandle handl, ScriptComponent& comp)
			{
				if (!comp.m_script.GetId().IsValid())
					return;

				ScriptAsset* pAsset = ResolveAsset(comp);

				if (pAsset == nullptr)
					return;

				ScriptInstance* pInstance = FindInstance(handl);

				if (pInstance != nullptr && pInstance->scriptId != comp.m_script.GetId())
				{
					StopInstance(*pInstance);
					m_instances.erase(handl.Index());
					pInstance = nullptr;
				}

				if (pInstance == nullptr)
					pInstance = CreateInstance(handl, comp, *pAsset);

				pInstance->lastSeenFrame = m_frame;

				if (pInstance->state == ScriptInstanceState::Running)
					UpdateInstance(*pInstance, comp, *pAsset, ctx.DeltaTime());
			});

		SweepInstances();
	}

	void ScriptSystem::OnFinalize()
	{
		EndSession(GetWorldService()->GetCurrentWorld());
	}

	void ScriptSystem::OnGroupsChanged(SystemGroup previous, SystemGroup current)
	{
		Scene* pScene = GetWorldService()->GetCurrentWorld();

		// Stop to play
		if (previous == SceneGroups::Edit && current == SceneGroups::Play)
		{
			BeginSession();
			return;
		}

		// Pause to stop
		if (previous == SceneGroups::Pause && current == SceneGroups::Edit)
		{
			EndSession(pScene);
			return;
		}

		// Play to stop
		if (previous == SceneGroups::Play && current == SceneGroups::Edit)
		{
			EndSession(pScene);
			return;
		}
	}

	void ScriptSystem::BeginSession()
	{
		m_context->Reset();
		m_instances.clear();
		m_classRefs.clear();
		m_frame = 0;
		m_inSession = true;
	}

	void ScriptSystem::EndSession(Scene* pScene)
	{
		if (!m_inSession)
			return;

		for (auto& [index, instance] : m_instances)
			StopInstance(instance);

		m_instances.clear();
		m_classRefs.clear();

		if (pScene != nullptr)
		{
			pScene->ForEach<ScriptComponent>([&](EntityHandle handl, ScriptComponent& comp)
				{
					comp.m_liveValues.Clear();
				});
		}

		m_context->Reset();
		m_inSession = false;
	}

	ScriptAsset* ScriptSystem::ResolveAsset(const ScriptComponent& comp)
	{
		const Guid& id = comp.m_script.GetId();

		if (!m_assetService->HasAsset(id))
			return nullptr;

		ScriptAsset* pAsset = m_assetService->FindAsset<ScriptAsset>(id);
		pAsset->LoadAsync();

		if (pAsset->GetResidencyState() != AssetResidency::Resident)
			return nullptr;

		return pAsset;
	}

	ScriptInstance* ScriptSystem::FindInstance(EntityHandle entity)
	{
		auto it = m_instances.find(entity.Index());

		if (it == m_instances.end())
			return nullptr;

		if (it->second.entity.Generation() != entity.Generation())
		{
			StopInstance(it->second);
			m_instances.erase(it);
			return nullptr;
		}

		return &it->second;
	}

	ScriptInstance* ScriptSystem::CreateInstance(EntityHandle entity, ScriptComponent& comp, const ScriptAsset& asset)
	{
		ScriptInstance& instance = m_instances[entity.Index()];
		instance = ScriptInstance();
		instance.entity = entity;
		instance.scriptId = comp.m_script.GetId();

		const i32 metaRef = LoadClass(asset);

		if (metaRef == ScriptInstance::InvalidRef)
		{
			instance.state = ScriptInstanceState::Failed;
			return &instance;
		}

		sol::state_view lua(m_context->GetState());

		sol::table self = lua.create_table();
		self[sol::metatable_key] = sol::table(lua.lua_state(), sol::ref_index(metaRef));
		self["entity"] = entity;

		const List<ScriptField>& fields = asset.GetFields();

		comp.m_liveValues.Clear();
		comp.m_liveValues.Reserve(fields.GetCount());

		for (const ScriptField& field : fields)
		{
			ScriptLiveValue& live = comp.m_liveValues.EmplaceBack();
			live.name = field.name;
			live.kind = field.kind;
			live.value = ResolveValue(comp, field);
			live.entity = FindEntityRef(comp, field.name);

			self[field.name] = ToLua(lua, field, live.value, live.entity);
		}

		self.push();
		instance.selfRef = luaL_ref(lua.lua_state(), LUA_REGISTRYINDEX);

		std::string error;

		if (!CallHook(self, "OnStart", error))
		{
			Terminal::Error(StringOps::GetName(this), "{} OnStart failed, {}", asset.GetChunkName(), error);
			instance.state = ScriptInstanceState::Failed;
		}

		return &instance;
	}

	i32 ScriptSystem::LoadClass(const ScriptAsset& asset)
	{
		const Guid& id = asset.GetPhysicalEntry().assetId;
		auto it = m_classRefs.find(id);

		if (it != m_classRefs.end())
			return it->second;

		sol::state_view lua(m_context->GetState());
		sol::protected_function_result result = lua.safe_script(asset.GetSource(), sol::script_pass_on_error, asset.GetChunkName(), sol::load_mode::text);

		if (!result.valid())
		{
			sol::error error = result;
			Terminal::Error(StringOps::GetName(this), "{}", error.what());
			m_classRefs[id] = ScriptInstance::InvalidRef;
			return ScriptInstance::InvalidRef;
		}

		sol::object returned = result.get<sol::object>();

		if (returned.get_type() != sol::type::table)
		{
			Terminal::Error(StringOps::GetName(this), "{} must return its class table", asset.GetChunkName());
			m_classRefs[id] = ScriptInstance::InvalidRef;
			return ScriptInstance::InvalidRef;
		}

		sol::table meta = lua.create_table_with("__index", returned);
		meta.push();

		const i32 metaRef = luaL_ref(lua.lua_state(), LUA_REGISTRYINDEX);
		m_classRefs[id] = metaRef;

		return metaRef;
	}

	void ScriptSystem::UpdateInstance(ScriptInstance& instance, ScriptComponent& comp, const ScriptAsset& asset, f32 deltaTime)
	{
		sol::state_view lua(m_context->GetState());
		sol::table self(lua.lua_state(), sol::ref_index(instance.selfRef));

		const List<ScriptField>& fields = asset.GetFields();
		const usize count = std::min(fields.GetCount(), comp.m_liveValues.GetCount());

		for (usize i = 0; i < count; ++i)
		{
			ScriptLiveValue& live = comp.m_liveValues[i];

			if (!live.dirty)
				continue;

			self[live.name] = ToLua(lua, fields[i], live.value, live.entity);
			live.dirty = false;
		}

		std::string error;

		if (!CallHook(self, "OnUpdate", error, deltaTime))
		{
			Terminal::Error(StringOps::GetName(this), "{} OnUpdate failed, {}", asset.GetChunkName(), error);
			instance.state = ScriptInstanceState::Failed;
			return;
		}

		for (usize i = 0; i < count; ++i)
			FromLua(self.get<sol::object>(comp.m_liveValues[i].name), comp.m_liveValues[i]);
	}

	void ScriptSystem::StopInstance(ScriptInstance& instance)
	{
		if (instance.selfRef == ScriptInstance::InvalidRef)
			return;

		lua_State* pState = m_context->GetState();

		if (instance.state == ScriptInstanceState::Running)
		{
			sol::table self(pState, sol::ref_index(instance.selfRef));
			std::string error;

			if (!CallHook(self, "OnStop", error))
				Terminal::Error(StringOps::GetName(this), "OnStop failed, {}", error);
		}

		luaL_unref(pState, LUA_REGISTRYINDEX, instance.selfRef);
		instance.selfRef = ScriptInstance::InvalidRef;
	}

	void ScriptSystem::SweepInstances()
	{
		for (auto it = m_instances.begin(); it != m_instances.end();)
		{
			if (it->second.lastSeenFrame == m_frame)
			{
				++it;
				continue;
			}

			StopInstance(it->second);
			it = m_instances.erase(it);
		}
	}

	const ScriptField* ScriptSystem::FindField(const List<ScriptField>& fields, std::string_view name)
	{
		for (const ScriptField& field : fields)
		{
			if (field.name == name)
				return &field;
		}

		return nullptr;
	}

	const ScriptFieldOverride* ScriptSystem::FindOverride(const ScriptComponent& comp, std::string_view name)
	{
		for (const ScriptFieldOverride& entry : comp.m_overrides)
		{
			if (entry.m_name == name)
				return &entry;
		}

		return nullptr;
	}

	const ScriptFieldValue& ScriptSystem::ResolveValue(const ScriptComponent& comp, const ScriptField& field)
	{
		const ScriptFieldOverride* pOverride = FindOverride(comp, field.name);

		if (pOverride == nullptr || pOverride->m_kind != field.kind)
			return field.defaultValue;

		return pOverride->m_value;
	}

	EntityHandle ScriptSystem::FindEntityRef(const ScriptComponent& comp, std::string_view name)
	{
		const usize count = std::min(comp.m_entityNames.GetCount(), comp.m_entityRefs.GetCount());

		for (usize i = 0; i < count; ++i)
		{
			if (comp.m_entityNames[i] == name)
				return comp.m_entityRefs[i];
		}

		return EntityHandle();
	}
}