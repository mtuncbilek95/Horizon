#pragma once

namespace Horizon
{
	enum class InputModifiers : u32
	{
		None = 0,
		Shift = 1 << 0,
		Control = 1 << 1,
		Alt = 1 << 2,
		Super = 1 << 2,
		CapsLock = 1 << 4,
		NumLock = 1 << 5,
	};
	GENERATE_FLAGS(InputModifiers);
}
