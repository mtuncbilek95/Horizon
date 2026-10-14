#pragma once

#include <Engine/Input/InputBinding.h>
#include <Runtime/Containers/List.h>
#include <Runtime/RTTR/Reflection.h>

#include <string>
#include <string_view>

namespace Horizon::Engine
{
	HENUM();
	enum class InputActionType
	{
		Button,
		Axis1D,
		Axis2D
	};

	HCLASS();
	struct ENGINE_API InputAction
	{
		HORIZON_DATA_REFLECT(InputAction);

		static constexpr u64 HashName(std::string_view name)
		{
			u64 hash = 14695981039346656037ull;

			for (const c8 character : name)
			{
				hash ^= u64(u8(character));
				hash *= 1099511628211ull;
			}

			return hash;
		}

		HFIELD();
		std::string name;

		HFIELD();
		InputActionType type = InputActionType::Button;

		HFIELD();
		b8 normalize = true;

		HFIELD();
		f32 pressThreshold = 0.5f;

		HFIELD();
		List<InputBinding> bindings;
	};
}