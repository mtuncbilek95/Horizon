#include "InputActionMapView.h"

#include <Editor/Domain/DomainFile.h>
#include <Editor/Renderer/EditorContext.h>
#include <Engine/Asset/Input/InputActionMapSerializer.h>
#include <Engine/Core/Engine.h>
#include <Engine/Reflection/ReflectionSystem.h>
#include <Runtime/Containers/StringHelper.h>
#include <Runtime/Input/GamepadButtons.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/PAL/File/File.h>
#include <Runtime/PAL/Window/KeyCode.h>
#include <Runtime/PAL/Window/MouseButton.h>
#include <Runtime/Serialization/JsonArchive.h>

#include <imgui.h>

namespace Horizon::Editor
{
	InputActionMapView::~InputActionMapView()
	{
	}

	void InputActionMapView::OnInvoke()
	{
		m_reflection = GetContext()->pEngine->GetReflectionSystem();
	}

	void InputActionMapView::OnRender(const Engine::EngineFrame& context)
	{
		DomainFile* pFile = GetConnectedFile();
		if (!pFile)
			return;

		if (pFile != m_loadedFile)
		{
			m_loadedFile = pFile;
			m_isLoaded = LoadSource(pFile);
		}

		if (!m_isLoaded)
		{
			ImGui::TextUnformatted("Input action map source could not be read");
			return;
		}

		if (ImGui::Button("Save"))
			SaveSource();

		ImGui::SameLine();

		if (ImGui::Button("Add Action"))
			m_actions.EmplaceBack().name = "NewAction";

		ImGui::SeparatorText("Actions");

		i64 removeIndex = -1;

		for (usize i = 0; i < m_actions.GetCount(); ++i)
		{
			Asset::InputAction& action = m_actions[i];

			ImGui::PushID(static_cast<i32>(i));

			const c8* pHeader = action.name.empty() ? "Unnamed" : action.name.c_str();

			if (ImGui::TreeNodeEx("##action", ImGuiTreeNodeFlags_DefaultOpen, "%s", pHeader))
			{
				DrawAction(action);

				if (ImGui::Button("Remove Action"))
					removeIndex = static_cast<i64>(i);

				ImGui::TreePop();
			}

			ImGui::PopID();
		}

		if (removeIndex >= 0)
			m_actions.RemoveAt(static_cast<usize>(removeIndex));
	}

	b8 InputActionMapView::OnCommand(ViewCommand command)
	{
		if (command != ViewCommand::Save)
			return false;

		if (!m_isLoaded || !m_connectedFile)
			return false;

		return SaveSource();
	}

	b8 InputActionMapView::LoadSource(DomainFile* pFile)
	{
		m_actions.Clear();

		const std::string& path = pFile->GetSourcePath();

		PAL::FileAccessRequest request = PAL::File::RequestAccess(path, PAL::FileOperationAccessPolicy::Read,
			PAL::FileOperationSharePolicy::SharedRead);

		if (!request.IsValid())
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} cannot be opened", path);
			return false;
		}

		std::string text;
		const b8 wasRead = PAL::File::ReadString(request, text);

		PAL::File::ReleaseAccess(request);

		if (!wasRead)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} cannot be read", path);
			return false;
		}

		JsonArchiveReader reader(text);

		if (!Asset::InputActionMapSerializer::Deserialize(m_actions, reader) || reader.HasError())
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} is not a valid input action map archive", path);
			return false;
		}

		return true;
	}

	b8 InputActionMapView::SaveSource()
	{
		JsonArchiveWriter writer;

		if (!Asset::InputActionMapSerializer::Serialize(m_actions, writer))
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Input action map could not be serialized");
			return false;
		}

		const std::string& path = m_connectedFile->GetSourcePath();

		PAL::FileAccessRequest request = PAL::File::RequestAccess(path, PAL::FileOperationAccessPolicy::Write,
			PAL::FileOperationSharePolicy::Exclusive);

		PAL::File::Truncate(request, 0);

		const b8 wasWritten = PAL::File::WriteString(request, writer.ToString());

		PAL::File::ReleaseAccess(request);

		if (!wasWritten)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} cannot be written", path);
			return false;
		}

		return true;
	}

	void InputActionMapView::DrawAction(Asset::InputAction& action)
	{
		c8 nameBuffer[128] = {};
		action.name.copy(nameBuffer, sizeof(nameBuffer) - 1);

		if (ImGui::InputText("Name", nameBuffer, sizeof(nameBuffer)))
			action.name = nameBuffer;

		i64 removeIndex = -1;

		for (usize i = 0; i < action.bindings.GetCount(); ++i)
		{
			ImGui::PushID(static_cast<i32>(i));

			DrawBinding(action.bindings[i]);

			ImGui::SameLine();

			if (ImGui::Button("X"))
				removeIndex = static_cast<i64>(i);

			ImGui::PopID();
		}

		if (removeIndex >= 0)
			action.bindings.RemoveAt(static_cast<usize>(removeIndex));

		if (ImGui::Button("Add Binding"))
			action.bindings.EmplaceBack();
	}

	void InputActionMapView::DrawBinding(Asset::InputBinding& binding)
	{
		u64 source = static_cast<u64>(binding.source);

		ImGui::SetNextItemWidth(180.0f);

		if (DrawEnumCombo("##source", Reflect::TypeOf<Asset::InputBindingSource>(), source))
		{
			binding.source = static_cast<Asset::InputBindingSource>(source);
			binding.code = 0;
		}

		ImGui::SameLine();
		ImGui::SetNextItemWidth(180.0f);

		DrawCode(binding);

		ImGui::SameLine();
		ImGui::SetNextItemWidth(160.0f);

		f32 scale[2] = { binding.scale.X(), binding.scale.Y() };

		if (ImGui::DragFloat2("##scale", scale, 0.01f))
			binding.scale.Set(scale[0], scale[1]);
	}

	b8 InputActionMapView::DrawCode(Asset::InputBinding& binding)
	{
		switch (binding.source)
		{
		case Asset::InputBindingSource::Key:
			return DrawEnumCombo("##code", Reflect::TypeOf<PAL::KeyCode>(), binding.code);
		case Asset::InputBindingSource::MouseButton:
			return DrawEnumCombo("##code", Reflect::TypeOf<PAL::MouseButton>(), binding.code);
		case Asset::InputBindingSource::GamepadButton:
			return DrawEnumCombo("##code", Reflect::TypeOf<Input::GamepadButtons>(), binding.code);
		default:
			break;
		}

		ImGui::BeginDisabled();
		ImGui::Button("-", ImVec2(180.0f, 0.0f));
		ImGui::EndDisabled();

		return false;
	}

	b8 InputActionMapView::DrawEnumCombo(const c8* pLabel, Reflect::TypeHandle enumType, u64& value)
	{
		Reflect::Type* pEnumType = m_reflection->GetType(enumType);

		if (!pEnumType)
		{
			ImGui::TextDisabled("unregistered enum");
			return false;
		}

		ReadOnlyList<const Reflect::EnumValue> values = pEnumType->GetEnumValues();

		if (values.GetCount() == 0)
		{
			ImGui::TextDisabled("empty enum");
			return false;
		}

		i32 currentIndex = -1;

		for (usize i = 0; i < values.GetCount(); i++)
		{
			if (values[i].value == static_cast<i64>(value))
			{
				currentIndex = static_cast<i32>(i);
				break;
			}
		}

		const c8* pPreview = currentIndex < 0 ? "Unknown" : values[currentIndex].name.c_str();

		if (!ImGui::BeginCombo(pLabel, pPreview))
			return false;

		b8 changed = false;

		for (usize i = 0; i < values.GetCount(); i++)
		{
			const b8 selected = static_cast<i32>(i) == currentIndex;

			if (ImGui::Selectable(values[i].name.c_str(), selected))
			{
				value = static_cast<u64>(values[i].value);
				changed = true;
			}

			if (selected)
				ImGui::SetItemDefaultFocus();
		}

		ImGui::EndCombo();
		return changed;
	}
}
