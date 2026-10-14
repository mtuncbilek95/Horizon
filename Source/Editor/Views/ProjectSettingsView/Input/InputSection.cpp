#include "InputSection.h"

#include <Editor/Font/IconsFontAwesome6.h>
#include <Editor/Project/ProjectContext.h>
#include <Engine/Core/Engine.h>
#include <Engine/Input/InputService.h>
#include <Engine/Reflection/ReflectionSystem.h>
#include <Runtime/Containers/StringOps.h>
#include <Runtime/Input/GamepadButtons.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/PAL/Window/KeyCode.h>
#include <Runtime/PAL/Window/MouseButton.h>

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

namespace Horizon::Editor
{
	namespace
	{
		constexpr f32 kNameWidth = 180.0f;
		constexpr f32 kComboWidth = 150.0f;
		constexpr f32 kNumberWidth = 70.0f;
	}

	void InputSection::OnInitialize()
	{
		auto* pEngine = GetContext()->pEngine;

		m_input = pEngine->RequestService<Engine::InputService>();

		auto* pProject = pEngine->RequestContext<ProjectContext>();
		if (pProject)
			m_filePath = pProject->GetSettingsPath() + "/GameInputSettings.hsettings";

		auto* pReflection = pEngine->GetReflectionSystem();
		m_sourceType = pReflection->GetType(Reflect::TypeOf<Engine::InputSource>());
		m_axisType = pReflection->GetType(Reflect::TypeOf<Engine::InputAxis>());
		m_actionTypeType = pReflection->GetType(Reflect::TypeOf<Engine::InputActionType>());
		m_keyCodeType = pReflection->GetType(Reflect::TypeOf<PAL::KeyCode>());
		m_mouseButtonType = pReflection->GetType(Reflect::TypeOf<PAL::MouseButton>());
		m_mouseAxisType = pReflection->GetType(Reflect::TypeOf<Engine::MouseAxisCode>());
		m_gamepadButtonType = pReflection->GetType(Reflect::TypeOf<Input::GamepadButtons>());
		m_gamepadAxisType = pReflection->GetType(Reflect::TypeOf<Engine::GamepadAxisCode>());

		if (!m_input)
		{
			Terminal::Error(StringOps::GetName(this), "InputService is not registered, input settings are unavailable");
			return;
		}

		if (!m_filePath.empty())
			m_input->LoadInputFromFile(m_filePath);

		m_map = m_input->GetMap();
		m_dirty = false;
	}

	void InputSection::OnRender(const Engine::EngineFrame& context)
	{
		if (!m_input)
		{
			ImGui::TextDisabled("InputService is unavailable");
			return;
		}

		RenderToolbar();
		ImGui::Separator();

		usize removeIndex = kInvalid64;

		for (usize i = 0; i < m_map.maps.GetCount(); i++)
		{
			ImGui::PushID(i32(i));

			b8 remove = false;
			RenderMap(m_map.maps[i], i, remove);

			if (remove)
				removeIndex = i;

			ImGui::PopID();
		}

		if (removeIndex != kInvalid64)
		{
			m_map.maps.RemoveAt(removeIndex);
			m_dirty = true;
		}

		if (ImGui::Button(ICON_FA_PLUS " Add Action Map"))
		{
			Engine::InputActionMap map = {};
			map.name = "NewMap";
			m_map.maps.PushBack(map);
			m_dirty = true;
		}
	}

	void InputSection::OnFinalize()
	{
		if (m_dirty)
			Save();
	}

	void InputSection::Apply()
	{
		m_input->SetMap(m_map);
		m_dirty = false;
	}

	void InputSection::Save()
	{
		Apply();

		if (m_filePath.empty())
		{
			Terminal::Error(StringOps::GetName(this), "No settings path is known, input map was applied but not saved");
			return;
		}

		m_input->SaveInputToFile(m_filePath);
	}

	void InputSection::Revert()
	{
		m_map = m_input->GetMap();
		m_dirty = false;
	}

	void InputSection::RenderToolbar()
	{
		ImGui::BeginDisabled(!m_dirty);

		if (ImGui::Button("Apply"))
			Apply();

		ImGui::SameLine();

		if (ImGui::Button(ICON_FA_FLOPPY_DISK " Save"))
			Save();

		ImGui::SameLine();

		if (ImGui::Button("Revert"))
			Revert();

		ImGui::EndDisabled();

		if (m_dirty)
		{
			ImGui::SameLine();
			ImGui::TextDisabled("(modified)");
		}
	}

	void InputSection::RenderMap(Engine::InputActionMap& map, usize mapIndex, b8& outRemove)
	{
		const std::string header = map.name.empty() ? "Unnamed Map" : map.name;
		const b8 open = ImGui::CollapsingHeader(header.c_str(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowOverlap);

		ImGui::SameLine(ImGui::GetContentRegionAvail().x - 20.0f);

		if (ImGui::SmallButton(ICON_FA_TRASH))
			outRemove = true;

		if (!open)
			return;

		ImGui::Indent();

		ImGui::SetNextItemWidth(kNameWidth);

		if (ImGui::InputText("Name", &map.name))
			m_dirty = true;

		ImGui::SameLine();

		if (ImGui::Checkbox("Enabled", &map.enabled))
			m_dirty = true;

		usize removeIndex = kInvalid64;

		for (usize i = 0; i < map.actions.GetCount(); i++)
		{
			ImGui::PushID(i32(i));

			b8 remove = false;
			RenderAction(map.actions[i], i, remove);

			if (remove)
				removeIndex = i;

			ImGui::PopID();
		}

		if (removeIndex != kInvalid64)
		{
			map.actions.RemoveAt(removeIndex);
			m_dirty = true;
		}

		if (ImGui::Button(ICON_FA_PLUS " Add Action"))
		{
			Engine::InputAction action = {};
			action.name = "NewAction";
			map.actions.PushBack(action);
			m_dirty = true;
		}

		ImGui::Unindent();
		ImGui::Spacing();
	}

	void InputSection::RenderAction(Engine::InputAction& action, usize actionIndex, b8& outRemove)
	{
		const std::string label = action.name.empty() ? "Unnamed Action" : action.name;
		const b8 open = ImGui::TreeNodeEx("##action", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowOverlap | ImGuiTreeNodeFlags_SpanAvailWidth, "%s", label.c_str());

		ImGui::SameLine(ImGui::GetContentRegionAvail().x - 20.0f);

		if (ImGui::SmallButton(ICON_FA_TRASH))
			outRemove = true;

		if (!open)
			return;

		ImGui::SetNextItemWidth(kNameWidth);

		if (ImGui::InputText("Name", &action.name))
			m_dirty = true;

		ImGui::SameLine();

		i64 type = i64(action.type);

		if (DrawEnumCombo("Type", m_actionTypeType, type, kComboWidth))
		{
			action.type = Engine::InputActionType(type);
			m_dirty = true;
		}

		if (action.type != Engine::InputActionType::Button)
		{
			ImGui::SameLine();

			if (ImGui::Checkbox("Normalize", &action.normalize))
				m_dirty = true;
		}

		ImGui::SameLine();
		ImGui::SetNextItemWidth(kNumberWidth);

		if (ImGui::DragFloat("Threshold", &action.pressThreshold, 0.01f, 0.0f, 1.0f))
			m_dirty = true;

		if (ImGui::BeginTable("##bindings", 7, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_BordersInnerH))
		{
			ImGui::TableSetupColumn("Source");
			ImGui::TableSetupColumn("Code");
			ImGui::TableSetupColumn("Axis");
			ImGui::TableSetupColumn("Scale");
			ImGui::TableSetupColumn("Player");
			ImGui::TableSetupColumn("");
			ImGui::TableSetupColumn("");
			ImGui::TableHeadersRow();

			usize removeIndex = kInvalid64;

			for (usize i = 0; i < action.bindings.GetCount(); i++)
			{
				ImGui::PushID(i32(i));
				ImGui::TableNextRow();

				b8 remove = false;
				RenderBinding(action.bindings[i], i, remove);

				if (remove)
					removeIndex = i;

				ImGui::PopID();
			}

			ImGui::EndTable();

			if (removeIndex != kInvalid64)
			{
				action.bindings.RemoveAt(removeIndex);
				m_dirty = true;
			}
		}

		if (ImGui::SmallButton(ICON_FA_PLUS " Add Binding"))
		{
			action.bindings.PushBack({});
			m_dirty = true;
		}

		ImGui::TreePop();
	}

	void InputSection::RenderBinding(Engine::InputBinding& binding, usize bindingIndex, b8& outRemove)
	{
		ImGui::TableNextColumn();

		i64 source = i64(binding.source);

		if (DrawEnumCombo("##source", m_sourceType, source, kComboWidth))
		{
			binding.source = Engine::InputSource(source);
			binding.code = 0;
			m_dirty = true;
		}

		ImGui::TableNextColumn();

		i64 code = i64(binding.code);

		if (DrawEnumCombo("##code", CodeTypeOf(binding.source), code, kComboWidth))
		{
			binding.code = u32(code);
			m_dirty = true;
		}

		ImGui::TableNextColumn();

		i64 axis = i64(binding.axis);

		if (DrawEnumCombo("##axis", m_axisType, axis, kNumberWidth))
		{
			binding.axis = Engine::InputAxis(axis);
			m_dirty = true;
		}

		ImGui::TableNextColumn();
		ImGui::SetNextItemWidth(kNumberWidth);

		if (ImGui::DragFloat("##scale", &binding.scale, 0.05f))
			m_dirty = true;

		ImGui::TableNextColumn();
		ImGui::SetNextItemWidth(kNumberWidth);

		i32 player = i32(binding.playerIndex);

		if (ImGui::InputInt("##player", &player, 0))
		{
			if (player < 0)
				player = 0;

			binding.playerIndex = u32(player);
			m_dirty = true;
		}

		ImGui::TableNextColumn();
		ImGui::TableNextColumn();

		if (ImGui::SmallButton(ICON_FA_TRASH))
			outRemove = true;
	}

	b8 InputSection::DrawEnumCombo(const char* label, const Reflect::Type* pEnumType, i64& value, f32 width)
	{
		ImGui::SetNextItemWidth(width);

		if (!pEnumType)
		{
			i32 raw = i32(value);

			if (!ImGui::InputInt(label, &raw, 0))
				return false;

			value = raw;
			return true;
		}

		ReadOnlyList<const Reflect::EnumValue> values = pEnumType->GetEnumValues();
		const char* pPreview = "?";

		for (usize i = 0; i < values.GetCount(); i++)
		{
			if (values[i].value == value)
			{
				pPreview = values[i].name.c_str();
				break;
			}
		}

		b8 changed = false;

		if (ImGui::BeginCombo(label, pPreview))
		{
			for (usize i = 0; i < values.GetCount(); i++)
			{
				const b8 selected = values[i].value == value;

				if (ImGui::Selectable(values[i].name.c_str(), selected))
				{
					value = values[i].value;
					changed = true;
				}

				if (selected)
					ImGui::SetItemDefaultFocus();
			}

			ImGui::EndCombo();
		}

		return changed;
	}

	const Reflect::Type* InputSection::CodeTypeOf(Engine::InputSource source) const
	{
		switch (source)
		{
		case Engine::InputSource::Key:
			return m_keyCodeType;
		case Engine::InputSource::MouseButton:
			return m_mouseButtonType;
		case Engine::InputSource::MouseAxis:
			return m_mouseAxisType;
		case Engine::InputSource::GamepadButton:
			return m_gamepadButtonType;
		case Engine::InputSource::GamepadAxis:
			return m_gamepadAxisType;
		default:
			return nullptr;
		}
	}
}