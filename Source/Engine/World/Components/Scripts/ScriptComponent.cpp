#include "ScriptComponent.h"

#include <Runtime/Definitions/Allocator.h>

namespace Horizon::Engine
{
	ScriptComponent::~ScriptComponent()
	{
		Memory::Allocator::Delete(m_script);
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

		Memory::Allocator::Delete(m_script);
		m_script = other.m_script;
		other.m_script = nullptr;

		return *this;
	}
}