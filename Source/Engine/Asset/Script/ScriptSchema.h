#pragma once

#include <Engine/Asset/Script/ScriptField.h>
#include <Runtime/Containers/List.h>

#include <string>
#include <string_view>

namespace Horizon::Engine
{
	class ReflectionSystem;

	class ScriptSchema
	{
	public:
		static constexpr i32 MaxInstructions = 1000000;

		static b8 Extract(std::string_view source, const std::string& chunkName, ReflectionSystem* pReflection, List<ScriptField>& outFields, std::string& outError);
	};
}