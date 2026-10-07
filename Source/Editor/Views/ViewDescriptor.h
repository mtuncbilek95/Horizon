#pragma once

#include <Editor/Views/DockZone.h>
#include <Editor/Views/EditorViewFlags.h>
#include <Runtime/RTTR/Reflection.h>
#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <string>

namespace Horizon::Editor
{
	struct EDITOR_API ViewDescriptor
	{
		std::string displayName;
		EditorViewFlags flags;
		DockZone dockZone;

		const Reflect::Type* pCoreType;
	};
}