#pragma once

#include <Editor/Views/EditorViewAttribute.h>
#include <Editor/Views/ViewObject.h>
#include <Editor/Font/IconsFontAwesome6.h>

namespace Horizon::TestLibrary
{
	HCLASS(Editor::EditorView[ICON_FA_BABY_CARRIAGE, "Test View", Editor::DockZone::Center, Editor::EditorViewFlags::Mutable | Editor::EditorViewFlags::MultiInstance]);
	class TestView final : public Editor::ViewObject
	{
		HORIZON_TYPE_REFLECT(TestView);
	public:
		TestView() = default;
		~TestView() = default;

		void OnInvoke() final;
		void OnRender(const Engine::EngineFrame& context) final;
	};
}