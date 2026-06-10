#include "WindowModule.h"

#include <Engine/Engine/Engine.h>
#include <Engine/CommandLine/CommandLineModule.h>

namespace Horizon
{
	WindowModule::WindowModule(const WindowDesc& desc) : m_desc(desc)
	{}

	WindowModule::~WindowModule()
	{}

	void WindowModule::OnAttach(Engine& engine)
	{
		IModule::OnAttach(engine);

		auto* cmdLineArg = engine.TryGetModule<CommandLineModule>();

		m_window = std::make_unique<Window>(m_desc);
	}

	void WindowModule::OnSync()
	{
		m_window->PollEvents();

		if (!m_window->IsActive())
			m_engine->RequestExit("Window closed!");
	}

	void WindowModule::OnDetach()
	{}
}