#pragma once

namespace Horizon::PAL
{
	enum class InputMessageType : u64
	{
		None,
		KeyDown,
		KeyUp,
		Char,
		MouseDown,
		MouseUp,
		MouseMove,
		MouseScroll,
		Resize,
		Move,
		Focus,
		LostFocus,
		Close,
		DropFiles
	};
}
