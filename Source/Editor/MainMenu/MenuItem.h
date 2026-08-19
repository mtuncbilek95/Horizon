#pragma once

#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	class Engine;
}

namespace Horizon::Editor
{
	class H_EXPORT MenuItem : public Reflect::Base
	{
		friend class MenuRegistry;
	public:
		virtual ~MenuItem() = default;

		virtual void OnExecute() = 0;

		Engine::Engine* GetEngine() const noexcept { return m_engine; }

	private:
		Engine::Engine* m_engine;
	};
}