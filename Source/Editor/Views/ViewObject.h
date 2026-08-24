#pragma once

#include <Runtime/RTTR/Reflection.h>
#include <string>

namespace Horizon::Engine
{
	class Engine;
}

namespace Horizon::Editor
{
	class ViewRegistry;

	class H_EXPORT ViewObject : public Reflect::Base
	{
		friend class ViewRegistry;
	public:
		virtual ~ViewObject() = default;

		virtual void OnInvoke() = 0;
		virtual void OnRender() = 0;

		ViewRegistry* GetRegistry() const { return m_holder; }

		Engine::Engine* GetEngine() const { return m_engine; }
		const std::string& GetDisplayName() const { return m_displayName; }

	protected:
		ViewRegistry* m_holder = nullptr;

		Engine::Engine* m_engine = nullptr;
		std::string m_displayName;
	};
}