#pragma once

#include <Engine/Asset/AssetTypeDesc.h>
#include <Runtime/Containers/List.h>
#include <Runtime/RTTR/Reflection.h>

#include <string>
#include <unordered_map>

namespace Horizon::Engine
{
	class Application;

	class H_EXPORT AssetTypeRegistry
	{
	public:
		void Bootstrap(Application* pEngine);

		const AssetTypeDesc& GetAssetDescriptor(Reflect::TypeHandle handl);

	private:
		List<AssetTypeDesc> m_registries;
		std::unordered_map<Reflect::TypeHandle, u32> m_registryAuxiliary;
	};
}