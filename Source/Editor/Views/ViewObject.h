#pragma once

#include <Runtime/RTTR/Reflection.h>

#include <string>

namespace Horizon::Engine
{
	class Application;
}

namespace Horizon::Editor
{
	class H_EXPORT ViewObject : public Reflect::Base
	{
		friend class ViewRegistry;
	public:
		virtual ~ViewObject() = default;

		virtual void OnInvoke() = 0;
		virtual void OnRender() = 0;

		Engine::Application* GetEngine() const { return m_engine; }

	protected:
		Engine::Application* m_engine;
		std::string m_displayName;
	};
}