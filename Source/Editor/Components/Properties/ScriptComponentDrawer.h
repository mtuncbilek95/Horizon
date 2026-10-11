#pragma once

#include <Editor/Components/PropertyDrawer.h>
#include <Editor/Components/Helpers/AssetPickerWidget.h>
#include <Editor/Components/Helpers/EntityPickerWidget.h>
#include <Engine/Asset/Script/ScriptField.h>
#include <Engine/Asset/Script/ScriptFieldOverride.h>
#include <Engine/World/Components/Scripts/ScriptComponent.h>
#include <Engine/World/Components/Scripts/ScriptLiveValue.h>
#include <Engine/World/ECS/Definitions.h>
#include <Runtime/Containers/List.h>

#include <string_view>

namespace Horizon::Engine
{
	class Scene;
}

namespace Horizon::Editor
{
	class DomainService;

	HCLASS();
	class EDITOR_API ScriptComponentDrawer final : public PropertyDrawer
	{
		HORIZON_TYPE_REFLECT(ScriptComponentDrawer);
	public:
		ScriptComponentDrawer() = default;
		~ScriptComponentDrawer() = default;

		Reflect::TypeHandle GetTargetType() const final { return Reflect::TypeOf<Engine::ScriptComponent>(); }
		b8 OnDraw(const Reflect::Field& field, void* pValue, const PropertyContext& ctx) final;

	private:
		b8 DrawScript(Engine::ScriptComponent& comp, const PropertyContext& ctx);
		b8 DrawEditField(Engine::ScriptComponent& comp, const Engine::ScriptField& field, DomainService* pDomain, Engine::Scene* pScene);
		b8 DrawLiveField(Engine::ScriptComponent& comp, const Engine::ScriptField& field, DomainService* pDomain, Engine::Scene* pScene);
		b8 DrawValue(const Engine::ScriptField& field, Engine::ScriptFieldValue& value, Engine::EntityHandle& entity, DomainService* pDomain, Engine::Scene* pScene);

		static void DrawLabel(const Engine::ScriptField& field, b8 isOverridden);
		static void DrawStatusRow(const c8* pLabel, const c8* pText);

		static const Engine::ScriptField* FindField(const List<Engine::ScriptField>& fields, std::string_view name);
		static Engine::ScriptFieldOverride* FindOverride(Engine::ScriptComponent& comp, std::string_view name);
		static Engine::ScriptFieldOverride& SetOverride(Engine::ScriptComponent& comp, const Engine::ScriptField& field);
		static void ResetOverride(Engine::ScriptComponent& comp, std::string_view name);
		static Engine::EntityHandle FindEntityRef(const Engine::ScriptComponent& comp, std::string_view name);
		static void SetEntityRef(Engine::ScriptComponent& comp, std::string_view name, Engine::EntityHandle entity);
		static void ResetEntityRef(Engine::ScriptComponent& comp, std::string_view name);
		static Engine::ScriptLiveValue* FindLiveValue(Engine::ScriptComponent& comp, std::string_view name);
		static void PruneStale(Engine::ScriptComponent& comp, const List<Engine::ScriptField>& fields);

	private:
		AssetPickerWidget m_assetPicker;
		EntityPickerWidget m_entityPicker;
	};
}