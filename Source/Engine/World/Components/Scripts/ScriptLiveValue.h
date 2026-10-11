#pragma once

#include <Engine/Asset/Script/ScriptField.h>
#include <Engine/World/ECS/Definitions.h>

#include <string>

namespace Horizon::Engine
{
	struct ScriptLiveValue
	{
		std::string name;
		ScriptFieldKind kind = ScriptFieldKind::Number;
		ScriptFieldValue value;
		EntityHandle entity;
		b8 dirty = false;
	};
}