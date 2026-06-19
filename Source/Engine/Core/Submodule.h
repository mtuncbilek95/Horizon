#pragma once

namespace Horizon
{
	class Engine;

	class Submodule
	{
	public:
		virtual ~Submodule() = default;

		virtual void OnAttach(Engine* engine) { m_engine = engine; }
		virtual void OnSync() {}
		virtual void OnDetach() {}

	protected:
		Engine* m_engine;
	};
}