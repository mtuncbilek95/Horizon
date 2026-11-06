#pragma once

#include <Editor/Views/EditorViewAttribute.h>
#include <Editor/Views/ViewObject.h>
#include <Editor/Font/IconsFontAwesome6.h>

namespace Horizon::Engine
{
	class PresentationService;
}

namespace Horizon::RHI
{
	class DescriptorHeap;
	class TextureView;
}

namespace Horizon::Editor
{
	HCLASS(EditorView[ICON_FA_MOUNTAIN_SUN, "Scene", DockZone::Center, EditorViewFlags::OpenOnStart]);
	class SceneView : public ViewObject
	{
		HORIZON_TYPE_REFLECT(SceneView);
	public:
		SceneView() = default;
		~SceneView();

		void OnInvoke() final;
		void OnRender(const Engine::EngineFrame& context) final;

		b8 IsFullBleed() const { return true; }

	private:
		Engine::PresentationService* m_presentation = nullptr;
		RHI::DescriptorHeap* m_resourceHeap = nullptr;
		RHI::TextureView* m_boundView = nullptr;
		u32 m_descriptorIndex = kInvalid32;
	};
}