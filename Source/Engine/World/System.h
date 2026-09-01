#pragma once

#include <Engine/Core/Engine.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	HCLASS();
	class H_EXPORT System : public Reflect::Base
	{
		friend class WorldService;
	public:
		virtual b8 OnInitialize() = 0;
		virtual void OnExecute(const EngineFrame& ctx) = 0;
		virtual void OnFinalize() = 0;

		Engine* GetEngine() const { return m_engine; }

	private:
		Engine* m_engine;
	};
}