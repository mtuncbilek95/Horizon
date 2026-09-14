#include "CreateEntityItem.h"

#include <Engine/World/Components/NameComponent.h>

namespace Horizon::Editor
{
	namespace
	{
		constexpr std::string_view kBaseName = "Entity";

		u32 ParseSuffix(std::string_view name)
		{
			if (name == kBaseName)
				return 0;

			if (!name.starts_with(kBaseName))
				return kInvalid32;

			std::string_view rest = name.substr(kBaseName.size());

			if (rest.size() < 4 || !rest.starts_with(" (") || !rest.ends_with(')'))
				return kInvalid32;

			std::string_view digits = rest.substr(2, rest.size() - 3);
			u32 value = 0;
			auto [ptr, ec] = std::from_chars(digits.data(), digits.data() + digits.size(), value);

			if (ec != std::errc() || ptr != digits.data() + digits.size())
				return kInvalid32;

			return value;
		}
	}

	void CreateEntityItem::OnExecute(SceneHierarchyContext& context)
	{
		u32 highest = kInvalid32;

		context.pCurrentScene->ForEach<Engine::NameComponent>([&](Engine::EntityHandle, const Engine::NameComponent& comp) 
			{
				u32 suffix = ParseSuffix(comp.m_name.ToString());

				if (suffix == kInvalid32)
					return;

				if (highest == kInvalid32 || suffix > highest)
					highest = suffix;
			});

		std::string name = highest == kInvalid32 ? std::string(kBaseName) : std::format("{} ({})", kBaseName, highest + 1);

		Engine::EntityHandle entt = context.pCurrentScene->AddEntity();
		auto* pName = context.pCurrentScene->AddComponent(entt, Engine::NameComponent());
		pName->m_name = NameId(name);
	}

	b8 CreateEntityItem::IsEnabled(const SceneHierarchyContext& context)
	{
		return context.pCurrentScene && context.selectedEntities.GetCount() == 0;
	}
}