#include "ScriptComponent.h"


namespace Horizon::Engine
{
	ScriptComponent::~ScriptComponent()
	{
		delete m_script;
		m_script = nullptr;
	}

	ScriptComponent::ScriptComponent(ScriptComponent&& other) noexcept : ComponentObject(std::move(other)), m_script(other.m_script)
	{
		other.m_script = nullptr;
	}

	ScriptComponent& ScriptComponent::operator=(ScriptComponent&& other) noexcept
	{
		if (this == &other)
			return *this;

		delete m_script;
		m_script = other.m_script;
		other.m_script = nullptr;

		return *this;
	}
}