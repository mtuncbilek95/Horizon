#pragma once

#include <Editor/Views/EditorViewAttribute.h>
#include <Editor/Views/ViewObject.h>
#include <Editor/Font/IconsFontAwesome6.h>

namespace Horizon::Editor
{
	HCLASS(EditorView[ICON_FA_BRUSH, "Material Graph", DockZone::Center, EditorViewFlags::OpenOnStart]);
	class EDITOR_API MaterialGraphView : public ViewObject
	{
		HORIZON_TYPE_REFLECT(MaterialGraphView);
	public:
		MaterialGraphView() = default;
		~MaterialGraphView();

		void OnInvoke() final;
		void OnRender(const Engine::EngineFrame& context) final;

		void OnLibraryRegistered(const Engine::ReflectionLibrary& library) final;
		void OnLibraryUnregistered(const Engine::ReflectionLibrary& library) final;

	private:
	};
}