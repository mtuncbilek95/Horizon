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
	class EditorContext;

	class H_EXPORT ViewObject : public Reflect::Base
	{
		friend class ViewRegistry;
	public:
		virtual ~ViewObject() = default;

		virtual void OnInvoke() = 0;
		virtual void OnRender() = 0;

		virtual b8 IsFullBleed() const { return false; }

		ViewRegistry* GetRegistry() const { return m_holder; }
		EditorContext* GetContext() const { return m_context; }

		const std::string& GetDisplayName() const { return m_displayName; }

	protected:
		ViewRegistry* m_holder = nullptr;
		EditorContext* m_context = nullptr;

		std::string m_displayName;
	};
}