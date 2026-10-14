#pragma once

#include <Engine/Input/InputAction.h>
#include <Runtime/Containers/List.h>
#include <Runtime/RTTR/Reflection.h>

#include <string>

namespace Horizon::Engine
{
	HCLASS();
	struct ENGINE_API InputActionMap
	{
		HORIZON_DATA_REFLECT(InputActionMap);

		HFIELD();
		std::string name;

		HFIELD();
		b8 enabled = true;

		HFIELD();
		List<InputAction> actions;
	};

	HCLASS();
	struct ENGINE_API InputMap
	{
		HORIZON_DATA_REFLECT(InputMap);

		HFIELD();
		List<InputActionMap> maps;
	};
}