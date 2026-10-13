#include "EntityHandlePropertyDrawer.h"

#include <Engine/Core/Engine.h>
#include <Engine/World/WorldService.h>

namespace Horizon::Editor
{
	b8 EntityHandlePropertyDrawer::OnDraw(const Reflect::Field& field, void* pValue, const PropertyContext& ctx)
	{
		auto* pEntity = static_cast<Engine::EntityHandle*>(pValue);
		auto* pWorld = ctx.pEngine->RequestService<Engine::WorldService>();

		return m_picker.Draw(pEntity, pWorld->GetCurrentWorld());
	}
}