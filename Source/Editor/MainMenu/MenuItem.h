#pragma once

namespace Horizon::Engine
{
	class Application;
}

namespace Horizon::Editor
{
	class H_EXPORT MenuItem
	{
		friend class MenuRegistry;
	public:
		virtual ~MenuItem() = default;

		virtual void OnExecute() = 0;

	protected:
		Engine::Application* m_engine;
	};
}