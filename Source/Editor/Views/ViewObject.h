#pragma once

namespace Horizon
{
	class Engine;

	class H_EXPORT ViewObject
	{
		friend class ViewRegistry;
	public:
		virtual ~ViewObject() = default;

		virtual void OnInvoke() = 0;
		virtual void OnRender() = 0;

		Engine* GetEngine() const { return m_engine; }

	protected:
		Engine* m_engine;
		std::string m_displayName;
	};
}