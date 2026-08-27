#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <imgui.h>

namespace Horizon::Editor
{
	struct EditState
	{
		u32 id = 0;
		f32 buffer[4] = {};

		void Capture(u32 target, const f32* pValues, u32 count)
		{
			id = target;

			for (u32 index = 0; index < count; index++)
				buffer[index] = pValues[index];
		}

		void Release() { id = 0; }
		b8 IsActive(u32 target) const { return id != 0 && id == target; }
	};
}