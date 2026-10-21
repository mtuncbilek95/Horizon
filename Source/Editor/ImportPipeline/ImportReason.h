#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::Editor
{
	enum class ImportReason : u8
	{
		Fresh,
		SourceChanged,
		SettingsChanged
	};
}