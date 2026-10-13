#pragma once

#include <Editor/Components/PropertyDrawer.h>
#include <Engine/World/Components/Scripts/ScriptComponent.h>

#include <string>
#include <string_view>

namespace Horizon::Editor
{
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
		b8 DrawPicker(Engine::ScriptComponent& comp, const PropertyContext& ctx, b8 isEditing);
		void DrawScriptFields(Engine::ScriptComponent& comp, const PropertyContext& ctx);

		static b8 MatchesFilter(std::string_view name, std::string_view filter);

	private:
		std::string m_searchFilter;
	};
}