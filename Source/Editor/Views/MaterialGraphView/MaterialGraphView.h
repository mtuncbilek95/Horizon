#pragma once

#include <Editor/Components/PropertyRenderer.h>
#include <Editor/Views/EditorViewAttribute.h>
#include <Editor/Views/ViewObject.h>
#include <Editor/Font/IconsFontAwesome6.h>
#include <Engine/Asset/Material/MaterialParameters.h>
#include <Engine/Asset/Material/MaterialProperties.h>

namespace Horizon::Engine
{
	class ReflectionSystem;
}

namespace Horizon::Editor
{
	HCLASS(EditorView[ICON_FA_BRUSH, "Material Graph", DockZone::Center, EditorViewFlags::Mutable | EditorViewFlags::MultiInstance]);
	class MaterialGraphView : public ViewObject
	{
		HORIZON_TYPE_REFLECT(MaterialGraphView);
	public:
		MaterialGraphView() = default;
		~MaterialGraphView();

		void OnInvoke() final;
		void OnRender(const Engine::EngineFrame& context) final;
		b8 OnCommand(ViewCommand command) final;

	private:
		b8 LoadSource(DomainFile* pFile);
		b8 SaveSource();

	private:
		Engine::ReflectionSystem* m_reflection = nullptr;
		PropertyRenderer m_propertyRenderer;

		Asset::MaterialProperties m_properties;
		Asset::MaterialParameters m_parameters;

		DomainFile* m_loadedFile = nullptr;
		b8 m_isLoaded = false;
	};
}