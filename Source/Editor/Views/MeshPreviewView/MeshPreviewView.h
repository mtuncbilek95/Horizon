#pragma once

#include <Editor/Components/Helpers/AssetPickerWidget.h>
#include <Editor/Components/PropertyRenderer.h>
#include <Editor/Views/EditorViewAttribute.h>
#include <Editor/Views/ViewObject.h>
#include <Editor/Font/IconsFontAwesome6.h>

namespace Horizon::Engine
{
	class AssetService;
}

namespace Horizon::Editor
{
	class DomainFile;
	class DomainService;

	HCLASS(EditorView[ICON_FA_CUBES, "Mesh Preview", DockZone::Center, EditorViewFlags::MultiInstance | EditorViewFlags::Mutable]);
	class MeshPreviewView : public ViewObject
	{
		HORIZON_TYPE_REFLECT(MeshPreviewView);
	public:
		MeshPreviewView() = default;
		~MeshPreviewView() = default;

		void OnInvoke() final;
		void OnRender(const Engine::EngineFrame& context) final;
		b8 OnCommand(ViewCommand command) final;

	private:
		void DrawImportSettings(DomainFile* pFile);
		b8 SaveSlots();

	private:
		Engine::AssetService* m_assetService = nullptr;
		DomainService* m_domain = nullptr;
		Reflect::Type* m_materialType = nullptr;
		Reflect::Type* m_settingsType = nullptr;

		AssetPickerWidget m_picker;
		PropertyRenderer m_properties;
	};
}