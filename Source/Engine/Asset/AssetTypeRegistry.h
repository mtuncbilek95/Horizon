#pragma once

#include <Engine/Asset/AssetTypeDesc.h>

#include <Runtime/Containers/List.h>

#include <string>
#include <unordered_map>

namespace Horizon
{
	class Engine;

	class H_EXPORT AssetTypeRegistry
	{
	public:
		void Bootstrap(Engine* pEngine);

		const AssetTypeDesc* ResolveByExtension(const std::string& ext) const;

	private:
		List<AssetTypeDesc> m_types;
		std::unordered_map<std::string, const AssetTypeDesc*> m_byExtension;
	};
}