#pragma once

#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	class Engine;
	class ReflectionSystem;
}

namespace Horizon::Editor
{
	struct PropertyContext
	{
		Engine::Engine* pEngine = nullptr;
		Engine::ReflectionSystem* pReflection = nullptr;
	};

	class H_EXPORT PropertyDrawer : public Reflect::Base
	{
	public:
		virtual ~PropertyDrawer() override = default;

		virtual Reflect::TypeHandle GetTargetType() const = 0;
		virtual b8 OnDraw(const Reflect::Field& field, void* pValue, const PropertyContext& ctx) = 0;
	};
}