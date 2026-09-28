#pragma once

#include <Editor/Components/ComponentDrawer.h>
#include <Engine/World/Components/Scripts/SpinTestComponent.h>

namespace Horizon::Editor
{
	HCLASS();
	class H_EXPORT SpinTestComponentDrawer : public ComponentDrawer
	{
		HORIZON_TYPE_REFLECT(SpinTestComponentDrawer);
	public:
		SpinTestComponentDrawer() = default;
		~SpinTestComponentDrawer() = default;

		void OnRender();

		Engine::ComponentTypeId GetComponentId() const { return Reflect::TypeOf<Engine::SpinTestComponent>(); }

	private:
		std::string m_currentLabel;
	};
}