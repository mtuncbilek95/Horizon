#pragma once

#include <Editor/Views/EditorViewAttribute.h>
#include <Editor/Views/ViewObject.h>
#include <Editor/Font/IconsFontAwesome6.h>
#include <Runtime/Containers/List.h>
#include <Runtime/RTTR/Reflection.h>

#include <string>

namespace Horizon::Editor
{
	class ProjectSection;

	HCLASS(EditorView[ICON_FA_SLIDERS, "Project Settings", DockZone::Center, EditorViewFlags::Mutable]);
	class EDITOR_API ProjectSettingsView : public ViewObject
	{
		HORIZON_TYPE_REFLECT(ProjectSettingsView);

		struct SectionEntry
		{
			ProjectSection* pSection = nullptr;
			const Reflect::Type* pType = nullptr;
			std::string displayName;
			i32 order = 0;
		};

	public:
		ProjectSettingsView() = default;
		~ProjectSettingsView();

		void OnInvoke() final;
		void OnRender(const Engine::EngineFrame& context) final;

		void OnLibraryRegistered(const Engine::ReflectionLibrary& library) final;
		void OnLibraryUnregistered(const Engine::ReflectionLibrary& library) final;

	private:
		b8 AddSectionType(const Reflect::Type* pNewSection);
		b8 RemoveSectionType(const Reflect::Type* pType);
		void ClearSections();

		void RenderSectionList();
		void RenderSelectedSection(const Engine::EngineFrame& context);

	private:
		List<SectionEntry> m_sections;
		usize m_selectedIndex = 0;
	};
}