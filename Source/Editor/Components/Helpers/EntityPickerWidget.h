#pragma once

#include <Engine/World/ECS/Definitions.h>

#include <string>

namespace Horizon::Engine
{
	class Scene;
}

namespace Horizon::Editor
{
	class EDITOR_API EntityPickerWidget
	{
	public:
		static constexpr const c8* PayloadName = "HZ_ENTITY";

	public:
		EntityPickerWidget() = default;
		~EntityPickerWidget() = default;

		b8 Draw(Engine::EntityHandle* pEntity, Engine::Scene* pScene);

	private:
		b8 DrawDropTarget(Engine::EntityHandle* pEntity, Engine::Scene* pScene);
		b8 DrawPicker(Engine::EntityHandle* pEntity, Engine::Scene* pScene);

		static std::string LabelOf(Engine::EntityHandle entity, Engine::Scene* pScene);

	private:
		std::string m_search;
	};
}