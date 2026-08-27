#pragma once

#include <Editor/Views/PropertyDrawer/EditState.h>
#include <Runtime/RTTR/Field.h>
#include <Runtime/RTTR/Type.h>

namespace Horizon::Editor
{
	struct PropertyContext
	{
		const Reflect::Field* pField = nullptr;
		void* pInstance = nullptr;
		const c8* pLabel = nullptr;
		EditState* pEditState = nullptr;
		u32 depth = 0;
	};
}