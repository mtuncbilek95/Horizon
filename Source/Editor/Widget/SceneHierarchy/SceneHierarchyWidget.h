#pragma once

#include <Editor/Widget/IWidget.h>

#include <Engine/ECS/EntityComponentSystem.h>
#include <Engine/ECS/WorldRegistry.h>
#include <Engine/ECS/EntityRegistry.h>
#include <Engine/ECS/ComponentRegistry.h>

namespace Horizon
{
	class WorldAsset;

	class SceneHierarchyWidget : public IWidget
	{
	public:
		void OnInvoke() final;
		void OnDraw() final;

	private:
		void AddNewEntity(WorldAsset& world);

	private:
		EntityComponentSystem* m_ecs = nullptr;
		WorldAsset* m_lastWorld;
		EntityHandle m_selected;
	};
}