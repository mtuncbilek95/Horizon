#pragma once

#include <Engine/Asset/Material/MaterialParameters.h>
#include <Engine/Asset/Material/MaterialProperties.h>
#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/RTTR/Reflection.h>
#include <Runtime/Serialization/Archive.h>

#include <string>

namespace Horizon::Engine
{
	class ReflectionSystem;
}

namespace Horizon::Asset
{
	class MaterialSerializer
	{
	public:
		static constexpr u32 Version = 1;

		static b8 Serialize(const MaterialProperties& properties, const MaterialParameters& parameters, Engine::ReflectionSystem* pReflection, IArchiveWriter& writer);
		static b8 Deserialize(MaterialProperties& properties, MaterialParameters& parameters, Engine::ReflectionSystem* pReflection, IArchiveReader& reader);

	private:
		static const Reflect::Type* ResolveType(void* pUserData, Reflect::TypeHandle handle);
		static const Reflect::Type* ResolveTypeByName(void* pUserData, const std::string& name);
	};
}