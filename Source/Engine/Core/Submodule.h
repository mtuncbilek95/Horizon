#pragma once

#include <Runtime/Reflection/Class.h>

H_GENERATE_REFLECTION;

namespace Horizon
{
	class Engine;

	HCLASS(Base);
	class Submodule : public Class
	{
		GENERATE_OBJECT;
	public:
		virtual ~Submodule() = default;

		virtual void OnAttach(Engine* engine) { m_engine = engine; }
		virtual void OnSync() {}
		virtual void OnDetach() {}

	protected:
		Engine* m_engine;
	};
}