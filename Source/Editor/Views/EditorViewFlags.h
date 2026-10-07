#pragma once

#include <Runtime/Definitions/BitwiseOperators.h>

namespace Horizon::Editor
{
	enum class EditorViewFlags : u32
	{
		None = 0,							// Immutable single panel that fucks your editor
		MultiInstance = 1 << 0,				// Such as MaterialGraphEditor, User can open multiple
		OpenOnStart = 1 << 1,				// Those are for the Embedded Editor elements, If you want something OpenOnStart, it cannot be Mutable. Engine will ignore this shit.
		Mutable = 1 << 2,					// View will have a close button on the right
	};
}