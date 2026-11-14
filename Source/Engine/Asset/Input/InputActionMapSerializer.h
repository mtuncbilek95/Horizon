#pragma once

#include <Engine/Asset/Input/InputAction.h>
#include <Runtime/Containers/Vector/List.h>
#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/Serialization/Archive.h>

namespace Horizon::Asset
{
	class InputActionMapSerializer
	{
	public:
		static constexpr u32 Version = 1;

		static b8 Serialize(const List<InputAction>& actions, IArchiveWriter& writer);
		static b8 Deserialize(List<InputAction>& actions, IArchiveReader& reader);
	};
}
