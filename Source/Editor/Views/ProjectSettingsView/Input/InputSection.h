#pragma once

#include <Editor/Views/ProjectSettingsView/ProjectSection.h>
#include <Editor/Views/ProjectSettingsView/ProjectSectionAttribute.h>
#include <Engine/Input/InputMap.h>
#include <Runtime/RTTR/Reflection.h>

#include <string>

namespace Horizon::Engine
{
	class InputService;
}

namespace Horizon::Editor
{
	HCLASS(ProjectSection["Input"]);
	class EDITOR_API InputSection : public ProjectSection
	{
		HORIZON_TYPE_REFLECT(InputSection);
	public:
		InputSection() = default;
		~InputSection() = default;

		void OnInitialize() final;
		void OnRender(const Engine::EngineFrame& context) final;
		void OnFinalize() final;

		void Apply();
		void Save();
		void Revert();

	private:
		void RenderToolbar();
		void RenderMap(Engine::InputActionMap& map, usize mapIndex, b8& outRemove);
		void RenderAction(Engine::InputAction& action, usize actionIndex, b8& outRemove);
		void RenderBinding(Engine::InputBinding& binding, usize bindingIndex, b8& outRemove);

		b8 DrawEnumCombo(const char* label, const Reflect::Type* pEnumType, i64& value, f32 width);
		const Reflect::Type* CodeTypeOf(Engine::InputSource source) const;

	private:
		Engine::InputService* m_input = nullptr;
		Engine::InputMap m_map;
		std::string m_filePath;
		b8 m_dirty = false;

		const Reflect::Type* m_sourceType = nullptr;
		const Reflect::Type* m_axisType = nullptr;
		const Reflect::Type* m_actionTypeType = nullptr;
		const Reflect::Type* m_keyCodeType = nullptr;
		const Reflect::Type* m_mouseButtonType = nullptr;
		const Reflect::Type* m_mouseAxisType = nullptr;
		const Reflect::Type* m_gamepadButtonType = nullptr;
		const Reflect::Type* m_gamepadAxisType = nullptr;
	};
}