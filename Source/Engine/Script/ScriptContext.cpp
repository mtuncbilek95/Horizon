#include "ScriptContext.h"

#include <Engine/Core/ModuleGraph.h>
#include <Engine/World/ECS/Definitions.h>

#include <Runtime/Containers/Guid.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/Math/Quat.h>
#include <Runtime/Math/Vec2f.h>
#include <Runtime/Math/Vec3f.h>

#include <sol/sol.hpp>

#include <string>

namespace Horizon::Engine
{
	struct ScriptContext::Impl
	{
		sol::state lua;
	};

	namespace
	{
		void RegisterConstructors(sol::state& lua)
		{
			lua.set_function("Range", [](f64 value, f64 min, f64 max)
				{
					return value;
				});

			lua.set_function("ReadOnly", [](sol::object value)
				{
					return value;
				});

			lua.set_function("Asset", [](const std::string& typeName)
				{
					return sol::lua_nil;
				});

			lua.set_function("Entity", []()
				{
					return EntityHandle();
				});

			lua.set_function("Vector2", [](sol::optional<f32> x, sol::optional<f32> y)
				{
					return Math::Vec2f(x.value_or(0.0f), y.value_or(0.0f));
				});

			lua.set_function("Vector3", [](sol::optional<f32> x, sol::optional<f32> y, sol::optional<f32> z)
				{
					return Math::Vec3f(x.value_or(0.0f), y.value_or(0.0f), z.value_or(0.0f));
				});

			lua.set_function("Quaternion", [](sol::optional<f32> x, sol::optional<f32> y, sol::optional<f32> z, sol::optional<f32> w)
				{
					return Math::Quat(x.value_or(0.0f), y.value_or(0.0f), z.value_or(0.0f), w.value_or(1.0f));
				});
		}

		void RegisterMath(sol::state& lua)
		{
			lua.new_usertype<Math::Vec2f>("Vec2f",
				sol::no_constructor,
				"X", sol::property([](const Math::Vec2f& v) { return v.X(); }, [](Math::Vec2f& v, f32 value) { v.X() = value; }),
				"Y", sol::property([](const Math::Vec2f& v) { return v.Y(); }, [](Math::Vec2f& v, f32 value) { v.Y() = value; }),
				sol::meta_function::addition, sol::resolve<Math::Vec2f(const Math::Vec2f&) const>(&Math::Vec2f::operator+),
				sol::meta_function::subtraction, sol::resolve<Math::Vec2f(const Math::Vec2f&) const>(&Math::Vec2f::operator-),
				sol::meta_function::multiplication, sol::overload(
					sol::resolve<Math::Vec2f(const Math::Vec2f&) const>(&Math::Vec2f::operator*),
					sol::resolve<Math::Vec2f(f32) const>(&Math::Vec2f::operator*)),
				sol::meta_function::division, sol::resolve<Math::Vec2f(f32) const>(&Math::Vec2f::operator/),
				sol::meta_function::unary_minus, sol::resolve<Math::Vec2f() const>(&Math::Vec2f::operator-),
				sol::meta_function::equal_to, &Math::Vec2f::operator==);

			lua.new_usertype<Math::Vec3f>("Vec3f",
				sol::no_constructor,
				"X", sol::property([](const Math::Vec3f& v) { return v.X(); }, [](Math::Vec3f& v, f32 value) { v.X() = value; }),
				"Y", sol::property([](const Math::Vec3f& v) { return v.Y(); }, [](Math::Vec3f& v, f32 value) { v.Y() = value; }),
				"Z", sol::property([](const Math::Vec3f& v) { return v.Z(); }, [](Math::Vec3f& v, f32 value) { v.Z() = value; }),
				sol::meta_function::addition, sol::resolve<Math::Vec3f(const Math::Vec3f&) const>(&Math::Vec3f::operator+),
				sol::meta_function::subtraction, sol::resolve<Math::Vec3f(const Math::Vec3f&) const>(&Math::Vec3f::operator-),
				sol::meta_function::multiplication, sol::overload(
					sol::resolve<Math::Vec3f(const Math::Vec3f&) const>(&Math::Vec3f::operator*),
					sol::resolve<Math::Vec3f(f32) const>(&Math::Vec3f::operator*)),
				sol::meta_function::division, sol::resolve<Math::Vec3f(f32) const>(&Math::Vec3f::operator/),
				sol::meta_function::unary_minus, sol::resolve<Math::Vec3f() const>(&Math::Vec3f::operator-),
				sol::meta_function::equal_to, &Math::Vec3f::operator==);

			lua.new_usertype<Math::Quat>("Quat",
				sol::no_constructor,
				"X", sol::readonly_property([](const Math::Quat& q) { return q.X(); }),
				"Y", sol::readonly_property([](const Math::Quat& q) { return q.Y(); }),
				"Z", sol::readonly_property([](const Math::Quat& q) { return q.Z(); }),
				"W", sol::readonly_property([](const Math::Quat& q) { return q.W(); }),
				sol::meta_function::multiplication, sol::overload(
					sol::resolve<Math::Quat(const Math::Quat&) const>(&Math::Quat::operator*),
					sol::resolve<Math::Vec3f(const Math::Vec3f&) const>(&Math::Quat::operator*)));
		}

		void RegisterHandles(sol::state& lua)
		{
			lua.new_usertype<EntityHandle>("EntityHandle",
				sol::no_constructor,
				"IsValid", &EntityHandle::IsValid,
				sol::meta_function::equal_to, &EntityHandle::operator==);

			lua.new_usertype<Guid>("AssetId",
				sol::no_constructor,
				"IsValid", &Guid::IsValid);
		}

		void RegisterPrint(sol::state& lua)
		{
			lua.set_function("print", [](sol::variadic_args args, sol::this_state state)
				{
					sol::state_view view(state);
					sol::protected_function toString = view["tostring"];
					std::string line;

					for (auto arg : args)
					{
						if (!line.empty())
							line += ' ';

						line += toString(arg).get<std::string>();
					}

					Terminal::Info("Script", "{}", line);
				});
		}
	}

	ModuleReport ScriptContext::OnInitialize()
	{
		CreateState();
		return ModuleReport();
	}

	void ScriptContext::OnFinalize()
	{
		DestroyState();
	}

	void ScriptContext::DeclareDependencies(ModuleGraph& graph)
	{
	}

	void ScriptContext::OnLibraryRegistered(const ReflectionLibrary& library)
	{
	}

	void ScriptContext::OnLibraryUnregistered(const ReflectionLibrary& library)
	{
	}

	void ScriptContext::Reset()
	{
		DestroyState();
		CreateState();
	}

	lua_State* ScriptContext::GetState() const
	{
		if (m_impl == nullptr)
			return nullptr;

		return m_impl->lua.lua_state();
	}

	void ScriptContext::CreateState()
	{
		m_impl = Memory::Allocator::Create<Impl>(Memory::CurrLoc());

		sol::state& lua = m_impl->lua;
		lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::string, sol::lib::table);

		RegisterConstructors(lua);
		RegisterMath(lua);
		RegisterHandles(lua);
		RegisterPrint(lua);
	}

	void ScriptContext::DestroyState()
	{
		if (m_impl == nullptr)
			return;

		Memory::Allocator::Delete(m_impl);
		m_impl = nullptr;
	}
}