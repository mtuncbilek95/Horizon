#pragma once

namespace Horizon
{
	class Engine;

	class IWidget
	{
	public:
		virtual ~IWidget() = default;

		void SetEngine(Engine* pEngine) { m_engine = pEngine; }
		Engine* GetEngine() const { return m_engine; }

		virtual void OnInvalidation() = 0;
		virtual void OnDraw() = 0;

	private:
		Engine* m_engine = nullptr;
	};
}