#pragma once

namespace Horizon
{
	class Engine;

	class IEditorItem
	{
	public:
		virtual ~IEditorItem() = default;

		void SetEngine(Engine* pEngine) { m_engine = pEngine; }
		Engine* GetEngine() const { return m_engine; }

	private:
		Engine* m_engine = nullptr;
	};
}