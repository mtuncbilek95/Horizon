#pragma once

#include <Engine/Asset/Script/ScriptField.h>
#include <Runtime/RTTR/Reflection.h>

#include <string>

namespace Horizon::Engine
{
	HCLASS();
	struct ScriptFieldOverride
	{
		HORIZON_DATA_REFLECT(ScriptFieldOverride);

		HFIELD();
		std::string m_name;

		HFIELD();
		ScriptFieldKind m_kind = ScriptFieldKind::Number;

		HFIELD();
		ScriptFieldValue m_value;
	};
}