#pragma once

#include <Editor/Widget/IWidget.h>

#include <Engine/ECS/EntityComponentSystem.h>
#include <Engine/ECS/WorldRegistry.h>
#include <Engine/ECS/EntityRegistry.h>
#include <Engine/ECS/ComponentRegistry.h>

namespace Horizon
{
	class SceneHierarchyWidget : public IWidget
	{
	public:
		void OnInvoke() final;
		void OnDraw() final;

	private:
		EntityComponentSystem* m_ecs = nullptr;
		EntityHandle m_selected;
	};
}