#include "CreateEntityItem.h"

#include <Engine/World/Components/NameComponent.h>

namespace Horizon::Editor
{
	void CreateEntityItem::OnExecute(SceneHierarchyContext& context)
	{
		if (!context.pCurrWorld)
		{
			Terminal::Error(StringOps::GetName(this), "Somehow current world is nullptr.");
			return;
		}

		Engine::EntityHandle newEntity = context.pCurrWorld->CreateEntity();
		auto* pNameComp = context.pCurrWorld->AddComponent(newEntity, Engine::NameComponent());

		u32 index = context.pCurrWorld->GetEntities().GetAliveCount();
		pNameComp->m_name = std::format("Entity {}", index);
	}

	b8 CreateEntityItem::IsEnabled(const SceneHierarchyContext& context)
	{
		return context.pCurrWorld && context.selectedEntities.IsEmpty();
	}
}