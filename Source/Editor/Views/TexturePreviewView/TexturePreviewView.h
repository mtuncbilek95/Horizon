#pragma once

#include <Editor/Views/EditorViewAttribute.h>
#include <Editor/Views/ViewObject.h>
#include <Editor/Font/IconsFontAwesome6.h>

namespace Horizon::Engine
{
	class AssetService;
	class PresentationService;
}

namespace Horizon::RHI
{
	class Device;
	class TextureView;
}

namespace Horizon::Editor
{
	HCLASS(EditorView[ICON_FA_MOUNTAIN_SUN, "Texture Preview", DockZone::Center, EditorViewFlags::MultiInstance | EditorViewFlags::Mutable]);
	class TexturePreviewView : public ViewObject
	{
		HORIZON_TYPE_REFLECT(TexturePreviewView);
	public:
		TexturePreviewView() = default;
		~TexturePreviewView();

		void OnInvoke() final;
		void OnRender(const Engine::EngineFrame& context) final;

		b8 IsFullBleed() const { return true; }

	private:
		Engine::AssetService* m_assetService = nullptr;
		RHI::Device* m_device = nullptr;
		RHI::TextureView* m_boundView = nullptr;
		u64 m_textureId = 0;
	};
}