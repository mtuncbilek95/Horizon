#include "MaterialSerializer.h"

#include <Engine/Reflection/ReflectionSystem.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/Serialization/Serializer.h>

namespace Horizon::Asset
{
	b8 MaterialSerializer::Serialize(const MaterialProperties& properties, const MaterialParameters& parameters, Engine::ReflectionSystem* pReflection, IArchiveWriter& writer)
	{
		const Reflect::Type* pType = pReflection->GetType(Reflect::TypeOf<MaterialParameters>());

		if (pType == nullptr)
		{
			Terminal::Error("MaterialSerializer", "MaterialParameters is not a reflected type");
			return false;
		}

		Serializer serializer(pReflection, ResolveType, ResolveTypeByName);

		writer.BeginObject();

		writer.Key("version");
		writer.WriteU64(Version);

		writer.Key("domain");
		writer.WriteU64(static_cast<u64>(properties.domain));

		writer.Key("twoSided");
		writer.WriteBool(properties.twoSided);

		writer.Key("parameters");
		serializer.Serialize(&parameters, *pType, writer);

		writer.EndObject();

		return true;
	}

	b8 MaterialSerializer::Deserialize(MaterialProperties& properties, MaterialParameters& parameters, Engine::ReflectionSystem* pReflection, IArchiveReader& reader)
	{
		const Reflect::Type* pType = pReflection->GetType(Reflect::TypeOf<MaterialParameters>());

		if (pType == nullptr)
		{
			Terminal::Error("MaterialSerializer", "MaterialParameters is not a reflected type");
			return false;
		}

		Serializer serializer(pReflection, ResolveType, ResolveTypeByName);

		reader.BeginObject();

		if (reader.Key("version") && reader.ReadU64() > Version)
		{
			Terminal::Error("MaterialSerializer", "Material archive is newer than the supported version {}", Version);
			reader.EndObject();
			return false;
		}

		if (reader.Key("domain"))
		{
			const u64 domain = reader.ReadU64();

			if (domain > static_cast<u64>(MaterialDomain::Transparent))
			{
				Terminal::Error("MaterialSerializer", "Material archive carries an unknown domain {}", domain);
				reader.EndObject();
				return false;
			}

			properties.domain = static_cast<MaterialDomain>(domain);
		}

		if (reader.Key("twoSided"))
			properties.twoSided = reader.ReadBool();

		if (!reader.Key("parameters"))
		{
			Terminal::Error("MaterialSerializer", "Material archive carries no parameters");
			reader.EndObject();
			return false;
		}

		serializer.Deserialize(&parameters, *pType, reader);

		reader.EndObject();

		return !reader.HasError();
	}

	const Reflect::Type* MaterialSerializer::ResolveType(void* pUserData, Reflect::TypeHandle handle)
	{
		return static_cast<Engine::ReflectionSystem*>(pUserData)->GetType(handle);
	}

	const Reflect::Type* MaterialSerializer::ResolveTypeByName(void* pUserData, const std::string& name)
	{
		return static_cast<Engine::ReflectionSystem*>(pUserData)->GetTypeByName(name);
	}
}