#include "StaticMeshLoadStrategy.h"

namespace Horizon::Engine
{
	Asset* StaticMeshLoadStrategy::Create(List<u8>&& payload)
	{
		return nullptr;
	}

	void StaticMeshLoadStrategy::Destroy(Asset* pAsset)
	{

	}

	b8 StaticMeshLoadStrategy::Activate(Asset* pAsset)
	{
		return false;
	}

	void StaticMeshLoadStrategy::Deactivate(Asset* pAsset)
	{

	}

	void StaticMeshLoadStrategy::OnRegister(Engine* pEngine)
	{

	}

	void StaticMeshLoadStrategy::OnUnregister(Engine* pEngine)
	{

	}

}