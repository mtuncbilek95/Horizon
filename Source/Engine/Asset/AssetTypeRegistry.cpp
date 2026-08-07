#include "AssetTypeRegistry.h"

#include <Engine/Asset/AssetTypeAttribute.h>
#include <Engine/Core/Engine.h>
#include <Engine/Module/ModuleContext.h>

#include <Runtime/Log/Terminal.h>

namespace Horizon
{
	void AssetTypeRegistry::Bootstrap(Engine* pEngine)
	{
		m_types.Clear();
		m_byExtension.clear();

		auto* moduleCtx = pEngine->GetModuleContext();
		List<Reflect::Type*> types = moduleCtx->GetTypeByAttribute(Reflect::TypeOf<AssetTypeAttribute>());

		for (auto* type : types)
		{
			auto* attr = type->GetCustomAttribute<AssetTypeAttribute>();

			AssetTypeDesc desc;
			desc.type = type;
			desc.origin = attr->GetOrigin();

			const std::string& csv = attr->GetExtensions();
			usize start = 0;

			while (start < csv.size())
			{
				const usize comma = csv.find(',', start);
				const usize end = (comma == std::string::npos) ? csv.size() : comma;

				desc.extensions.PushBack(csv.substr(start, end - start));
				start = end + 1;
			}

			m_types.PushBack(std::move(desc));
		}

		for (const auto& desc : m_types)
		{
			for (const auto& ext : desc.extensions)
				m_byExtension[ext] = &desc;
		}
	}

	const AssetTypeDesc* AssetTypeRegistry::ResolveByExtension(const std::string& ext) const
	{
		auto it = m_byExtension.find(ext);

		if (it == m_byExtension.end())
		{
			Terminal::Warn("AssetTypeRegistry", "{} has no asset type", ext);
			return nullptr;
		}

		return it->second;
	}
}