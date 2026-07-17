#pragma once

#include <Editor/Widget/IWidget.h>
#include <Editor/Widget/WidgetAttribute.h>
#include <Editor/Widget/DockLayout.h>
#include <Editor/Font/IconsFontAwesome6.h>

#include <Engine/ECS/EntityComponentSystem.h>
#include <Engine/ECS/WorldRegistry.h>
#include <Engine/ECS/EntityRegistry.h>
#include <Engine/ECS/ComponentRegistry.h>

#include "SceneHierarchyWidget.reflected.h"

namespace Horizon
{
	class WorldAsset;

	HCLASS(WidgetTypeAttribute["Scene Hierarchy", ICON_FA_SITEMAP, DockLayout::Left, true])
	class SceneHierarchyWidget : public IWidget
	{
		HORIZON_REFLECT;
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