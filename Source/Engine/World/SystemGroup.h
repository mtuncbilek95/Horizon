#pragma once

#include <Runtime/Definitions/BitwiseOperators.h>
#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::Engine
{
	enum class SystemGroup : u32
	{
		None = 0,
		General = 1 << 0,
		Render = 1 << 1,
		Script = 1 << 2,
		Audio = 1 << 3,
		Physics = 1 << 4,
		Editor = 1 << 5
	};

	namespace SceneGroups
	{
		constexpr SystemGroup Play = SystemGroup::General | SystemGroup::Render | SystemGroup::Script | SystemGroup::Audio | SystemGroup::Physics;
		constexpr SystemGroup Pause = SystemGroup::General | SystemGroup::Render;
		constexpr SystemGroup Edit = SystemGroup::General | SystemGroup::Render | SystemGroup::Editor;
		constexpr SystemGroup Standalone = Play;
	}
}