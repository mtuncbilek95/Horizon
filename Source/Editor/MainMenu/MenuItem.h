#pragma once

#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	class Application;
}

namespace Horizon::Editor
{
	class H_EXPORT MenuItem : public Reflect::Base
	{
		friend class MenuRegistry;
	public:
		virtual ~MenuItem() = default;

		virtual void OnExecute() = 0;

	protected:
		Engine::Application* m_engine;
	};
}