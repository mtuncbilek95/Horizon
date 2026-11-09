#include "Material.h"

namespace Horizon::Asset
{
	void Material::LoadAsync()
	{
	}

	void Material::UnloadAsync()
	{
	}

	MaterialUsePermit Material::BeginUse()
	{
		return MaterialUsePermit();
	}

	void Material::EndUse(MaterialUsePermit& permit)
	{
	}

	void Material::OnPropertiesChanged(ReadOnlyList<u8> propertyBytes)
	{
	}

	void Material::ReleaseUse()
	{
	}
}