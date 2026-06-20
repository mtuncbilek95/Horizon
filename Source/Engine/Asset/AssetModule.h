#pragma once

#include <Engine/Core/Submodule.h>
#include <Engine/Asset/AssetMetadata.h>

#include <unordered_map>

namespace Horizon
{
	class AssetModule final : public Submodule
	{
	public:
		void OnAttach(Engine* pEngine) final;
		void OnSync() final;
		void OnDetach() final;

		AssetProperties* Create(std::string typeName);

	private:
		template<typename T>
		void RegisterType(std::string name)
		{
			u32 id = TypeId<T>();
			auto it = m_registeredTypes.find(id);

			if (it != m_registeredTypes.end())
			{
				Terminal::Warn("AssetModule", "Type - {} has been already created. Ignoring...", typeid(T).name());
				return;
			}

			Terminal::Info("AssetModule", "Type - {} has been created.", typeid(T).name());
			m_typeNameToId[name] = id;
			m_registeredTypes[TypeId<T>()] = []() -> AssetProperties*
				{
					return Allocator::Create<T>(CurrLoc());
				};
		}

	private:
		std::unordered_map<u32, PropertyFactory> m_registeredTypes;
		std::unordered_map<std::string, u32> m_typeNameToId;
	};
}