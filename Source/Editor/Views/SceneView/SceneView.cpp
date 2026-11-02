#include "SceneView.h"

#include <Editor/Renderer/EditorContext.h>
#include <Editor/Renderer/Utils/ImGuiUtils.h>
#include <Editor/Models/SelectionModel.h>

#include <Engine/Core/Engine.h>

#include <Runtime/Containers/StringHelper.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/Math/Mat4f.h>
#include <Runtime/Math/Quat.h>
#include <Runtime/Math/Scalar.h>
#include <Runtime/Math/Vec3f.h>

#include <imgui.h>
#include <ImGuizmo.h>
#include <string_view>

namespace Horizon::Editor
{
	void SceneView::OnInvoke()
	{
	}

	void SceneView::OnRender(const Engine::EngineFrame& context)
	{
	}

	void SceneView::OnLibraryRegistered(const Engine::ReflectionLibrary& library)
	{
	}

	void SceneView::OnLibraryUnregistered(const Engine::ReflectionLibrary& library)
	{
	}
}