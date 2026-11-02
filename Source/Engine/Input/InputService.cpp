#include "InputService.h"

#include <Engine/Core/Engine.h>
#include <Engine/Core/ModuleGraph.h>
#include <Engine/Reflection/ReflectionSystem.h>
#include <Engine/World/WorldService.h>

namespace Horizon::Engine
{
	ModuleReport InputService::OnInitialize()
	{
		m_devices = UniquePtr<Input::Instance>(Input::CreateInstance());

		if (!m_devices)
			return ModuleReport("Input backend could not be created");

		return ModuleReport();
	}

	void InputService::OnExecute(const EngineFrame& ctx)
	{
		m_devices->UpdateInputs();
	}

	void InputService::OnFinalize()
	{
		m_devices = nullptr;
	}

	void InputService::DeclareDependencies(ModuleGraph& graph)
	{
		graph.Precedes<WorldService>();
	}

	void InputService::OnLibraryRegistered(const ReflectionLibrary& library)
	{
	}

	void InputService::OnLibraryUnregistered(const ReflectionLibrary& library)
	{
	}
}