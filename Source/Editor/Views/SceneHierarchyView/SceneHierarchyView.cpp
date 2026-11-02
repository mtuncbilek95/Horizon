#include "SceneHierarchyView.h"

namespace Horizon::Editor
{
	void SceneHierarchyView::OnInvoke()
	{
	}

	void SceneHierarchyView::OnRender(const Engine::EngineFrame& context)
	{
	}

	b8 SceneHierarchyView::OnCommand(ViewCommand command)
	{
		if (command != ViewCommand::Save)
			return false;

		return true;
	}

	void SceneHierarchyView::OnLibraryRegistered(const Engine::ReflectionLibrary& library)
	{
	}

	void SceneHierarchyView::OnLibraryUnregistered(const Engine::ReflectionLibrary& library)
	{
	}
}