#include "SceneView.h"

#include <Editor/Renderer/EditorContext.h>
#include <Editor/Renderer/Utils/ImGuiUtils.h>
#include <Editor/Models/SelectionModel.h>

#include <Engine/Core/Engine.h>
#include <Engine/Graphics/GraphicsContext.h>
#include <Engine/Presentation/PresentationService.h>

#include <Runtime/Containers/StringHelper.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/Math/Mat4f.h>
#include <Runtime/Math/Quat.h>
#include <Runtime/Math/Scalar.h>
#include <Runtime/Math/Vec2u.h>
#include <Runtime/Math/Vec3f.h>
#include <Runtime/RHI/Descriptor/DescriptorHeap.h>

#include <imgui.h>
#include <ImGuizmo.h>
#include <string_view>

namespace Horizon::Editor
{
	SceneView::~SceneView()
	{
		if (m_resourceHeap)
			m_resourceHeap->Free(m_descriptorIndex);
	}

	void SceneView::OnInvoke()
	{
		m_presentation = GetContext()->pEngine->RequestService<Engine::PresentationService>();
		if (!m_presentation)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "PresentationService is missing, scene will not be displayed");
			return;
		}

		auto* pGraphics = GetContext()->pEngine->RequestContext<Engine::GraphicsContext>();
		if (!pGraphics)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "GraphicsContext is missing, scene will not be displayed");
			return;
		}

		m_resourceHeap = pGraphics->GetResourceHeap();
		m_descriptorIndex = m_resourceHeap->Allocate();
	}

	void SceneView::OnRender(const Engine::EngineFrame& context)
	{
		if (!m_presentation || m_descriptorIndex == kInvalid32)
			return;

		const ImVec2 available = ImGui::GetContentRegionAvail();
		if (available.x < 1.0f || available.y < 1.0f)
			return;

		m_presentation->RequestSceneExtent(Math::Vec2u(available.x, available.y));

		RHI::TextureView* pView = m_presentation->GetSceneView();
		if (pView != m_boundView)
		{
			m_resourceHeap->Write(m_descriptorIndex, RHI::DescriptorType::Texture, pView);
			m_boundView = pView;
		}

		ImGui::Image((ImTextureID)m_resourceHeap->GetGpuHandle(m_descriptorIndex), available);
	}

	void SceneView::OnLibraryRegistered(const Engine::ReflectionLibrary& library)
	{
	}

	void SceneView::OnLibraryUnregistered(const Engine::ReflectionLibrary& library)
	{
	}
}