#pragma once

#include <Engine/World/ECS/ComponentIdAttribute.h>
#include <Engine/World/ECS/ComponentObject.h>
#include <Runtime/RTTR/Reflection.h>
#include <Runtime/RTTR/Attributes/ReadOnlyAttribute.h>
#include <Runtime/Math/Vec2f.h>

namespace Horizon::World::Component
{
	HCLASS(Engine::ComponentId["CameraViewport", "Camera"]);
	class CameraViewport final : public Engine::ComponentObject
	{
		HORIZON_TYPE_REFLECT(CameraViewport);
	public:
		CameraViewport() = default;
		~CameraViewport() = default;

		HFIELD(ReadOnly);
		Math::Vec2f m_targetScreen = { 0.f, 0.f };
	};
}