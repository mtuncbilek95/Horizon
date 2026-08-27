#pragma once

#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	class Engine;
	class World;

	HCLASS();
	class H_EXPORT System : public Reflect::Base
	{
		friend class WorldService;
	public:
		System() = default;
		virtual ~System() = default;

		virtual b8 OnInitialize() = 0;
		virtual void OnExecute(World& world) = 0;
		virtual void OnFinalize() = 0;

		Engine* GetEngine() const { return m_engine; }

	private:
		Engine* m_engine = nullptr;
	};
}