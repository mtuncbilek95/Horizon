#pragma once

#include <Runtime/RTTR/Field.h>

#include <imgui.h>

namespace Horizon::Editor
{
	struct InspectorWidgets
	{
		static void BeginRow(const c8* pLabel, const Reflect::Field* pField);
		static b8 DrawVec3(const c8* pLabel, f32* pValues, b8* pActive = nullptr);
		static b8 DrawAxisField(const c8* pAxis, f32* pValue, u32 accentColor, f32 width);
	};
}