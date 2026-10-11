#include "ScriptContext.h"

#include <Engine/Core/Engine.h>
#include <Engine/Core/ModuleGraph.h>
#include <Engine/Reflection/ReflectionSystem.h>
#include <Engine/World/ECS/ComponentObject.h>
#include <Engine/World/ECS/Definitions.h>
#include <Engine/World/ECS/Scene.h>

#include <Runtime/Containers/Guid.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/Math/Quat.h>
#include <Runtime/Math/Scalar.h>
#include <Runtime/Math/Vec2f.h>
#include <Runtime/Math/Vec3f.h>
#include <Runtime/RTTR/Attributes/ReadOnlyAttribute.h>

#include <sol/sol.hpp>

#include <string>
#include <unordered_map>

namespace Horizon::Engine
{
	namespace
	{
		struct BindingState
		{
			Scene* pScene = nullptr;
			ReflectionSystem* pReflection = nullptr;
			std::unordered_map<Reflect::TypeHandle, std::unordered_map<std::string, const Reflect::Field*>> fields;
		};

		struct ComponentProxy
		{
			EntityHandle entity;
			Reflect::TypeHandle type;
		};
	}

	struct ScriptContext::Impl
	{
		sol::state lua;
		BindingState bindings;
	};

	namespace
	{
		const Reflect::Field* FindReflectedField(BindingState& state, const Reflect::Type& type, const std::string& name)
		{
			auto& lookup = state.fields[type.GetTypeId()];

			if (lookup.empty())
			{
				for (const Reflect::Field& field : type.GetFields())
					lookup[field.GetName()] = &field;
			}

			auto it = lookup.find(name);

			if (it == lookup.end())
				return nullptr;

			return it->second;
		}

		ComponentObject* ResolveComponent(lua_State* pLua, BindingState& state, const ComponentProxy& proxy, const Reflect::Type*& outType)
		{
			if (state.pScene == nullptr)
				luaL_error(pLua, "No active world for component access");

			if (!state.pScene->GetEntities().IsAlive(proxy.entity))
				luaL_error(pLua, "Entity of this component no longer exists");

			outType = state.pReflection->GetType(proxy.type);
			ComponentObject* pComponent = state.pScene->FindComponent(proxy.entity, proxy.type);

			if (outType == nullptr || pComponent == nullptr)
				luaL_error(pLua, "Component no longer exists on this entity");

			return pComponent;
		}

		sol::object ReadReflected(sol::state_view& lua, const Reflect::Field& field, const void* pComponent)
		{
			if (field.GetMode() != Reflect::TypeMode::Compose)
				return sol::make_object(lua, sol::lua_nil);

			const void* pValue = field.GetValue(pComponent);
			const Reflect::TypeHandle typeId = field.GetTypeId();

			if (typeId == Reflect::TypeOf<EntityHandle>())
				return sol::make_object(lua, *static_cast<const EntityHandle*>(pValue));

			if (typeId == Reflect::TypeOf<Math::Vec2f>())
				return sol::make_object(lua, *static_cast<const Math::Vec2f*>(pValue));

			if (typeId == Reflect::TypeOf<Math::Vec3f>())
				return sol::make_object(lua, *static_cast<const Math::Vec3f*>(pValue));

			if (typeId == Reflect::TypeOf<Math::Quat>())
				return sol::make_object(lua, *static_cast<const Math::Quat*>(pValue));

			if (typeId == Reflect::TypeOf<Guid>())
				return sol::make_object(lua, *static_cast<const Guid*>(pValue));

			const Reflect::TypeKind kind = field.GetKind() == Reflect::TypeKind::Enum ? field.GetUnderlyingKind() : field.GetKind();

			switch (kind)
			{
			case Reflect::TypeKind::Boolean:    return sol::make_object(lua, *static_cast<const b8*>(pValue) != 0);
			case Reflect::TypeKind::Signed8:    return sol::make_object(lua, static_cast<i64>(*static_cast<const i8*>(pValue)));
			case Reflect::TypeKind::Signed16:   return sol::make_object(lua, static_cast<i64>(*static_cast<const i16*>(pValue)));
			case Reflect::TypeKind::Signed32:   return sol::make_object(lua, static_cast<i64>(*static_cast<const i32*>(pValue)));
			case Reflect::TypeKind::Signed64:   return sol::make_object(lua, *static_cast<const i64*>(pValue));
			case Reflect::TypeKind::Unsigned8:  return sol::make_object(lua, static_cast<i64>(*static_cast<const u8*>(pValue)));
			case Reflect::TypeKind::Unsigned16: return sol::make_object(lua, static_cast<i64>(*static_cast<const u16*>(pValue)));
			case Reflect::TypeKind::Unsigned32: return sol::make_object(lua, static_cast<i64>(*static_cast<const u32*>(pValue)));
			case Reflect::TypeKind::Unsigned64: return sol::make_object(lua, static_cast<i64>(*static_cast<const u64*>(pValue)));
			case Reflect::TypeKind::Float32:    return sol::make_object(lua, static_cast<f64>(*static_cast<const f32*>(pValue)));
			case Reflect::TypeKind::Float64:    return sol::make_object(lua, *static_cast<const f64*>(pValue));
			case Reflect::TypeKind::String:     return sol::make_object(lua, *static_cast<const std::string*>(pValue));
			default:
				return sol::make_object(lua, sol::lua_nil);
			}
		}

		b8 WriteReflected(const Reflect::Field& field, void* pComponent, const sol::object& value)
		{
			if (field.GetMode() != Reflect::TypeMode::Compose)
				return false;

			void* pValue = field.GetValue(pComponent);
			const Reflect::TypeHandle typeId = field.GetTypeId();
			const sol::type type = value.get_type();

			if (typeId == Reflect::TypeOf<EntityHandle>())
			{
				if (type == sol::type::lua_nil)
				{
					*static_cast<EntityHandle*>(pValue) = EntityHandle();
					return true;
				}

				if (!value.is<EntityHandle>())
					return false;

				*static_cast<EntityHandle*>(pValue) = value.as<EntityHandle>();
				return true;
			}

			if (typeId == Reflect::TypeOf<Math::Vec2f>())
			{
				if (!value.is<Math::Vec2f>())
					return false;

				*static_cast<Math::Vec2f*>(pValue) = value.as<Math::Vec2f>();
				return true;
			}

			if (typeId == Reflect::TypeOf<Math::Vec3f>())
			{
				if (!value.is<Math::Vec3f>())
					return false;

				*static_cast<Math::Vec3f*>(pValue) = value.as<Math::Vec3f>();
				return true;
			}

			if (typeId == Reflect::TypeOf<Math::Quat>())
			{
				if (!value.is<Math::Quat>())
					return false;

				*static_cast<Math::Quat*>(pValue) = value.as<Math::Quat>();
				return true;
			}

			if (typeId == Reflect::TypeOf<Guid>())
			{
				if (type == sol::type::lua_nil)
				{
					*static_cast<Guid*>(pValue) = Guid();
					return true;
				}

				if (!value.is<Guid>())
					return false;

				*static_cast<Guid*>(pValue) = value.as<Guid>();
				return true;
			}

			const Reflect::TypeKind kind = field.GetKind() == Reflect::TypeKind::Enum ? field.GetUnderlyingKind() : field.GetKind();

			if (kind == Reflect::TypeKind::Boolean)
			{
				if (type != sol::type::boolean)
					return false;

				*static_cast<b8*>(pValue) = value.as<bool>();
				return true;
			}

			if (kind == Reflect::TypeKind::String)
			{
				if (type != sol::type::string)
					return false;

				*static_cast<std::string*>(pValue) = value.as<std::string>();
				return true;
			}

			if (type != sol::type::number)
				return false;

			switch (kind)
			{
			case Reflect::TypeKind::Signed8:    *static_cast<i8*>(pValue) = static_cast<i8>(value.as<i64>()); return true;
			case Reflect::TypeKind::Signed16:   *static_cast<i16*>(pValue) = static_cast<i16>(value.as<i64>()); return true;
			case Reflect::TypeKind::Signed32:   *static_cast<i32*>(pValue) = static_cast<i32>(value.as<i64>()); return true;
			case Reflect::TypeKind::Signed64:   *static_cast<i64*>(pValue) = value.as<i64>(); return true;
			case Reflect::TypeKind::Unsigned8:  *static_cast<u8*>(pValue) = static_cast<u8>(value.as<i64>()); return true;
			case Reflect::TypeKind::Unsigned16: *static_cast<u16*>(pValue) = static_cast<u16>(value.as<i64>()); return true;
			case Reflect::TypeKind::Unsigned32: *static_cast<u32*>(pValue) = static_cast<u32>(value.as<i64>()); return true;
			case Reflect::TypeKind::Unsigned64: *static_cast<u64*>(pValue) = static_cast<u64>(value.as<i64>()); return true;
			case Reflect::TypeKind::Float32:    *static_cast<f32*>(pValue) = static_cast<f32>(value.as<f64>()); return true;
			case Reflect::TypeKind::Float64:    *static_cast<f64*>(pValue) = value.as<f64>(); return true;
			default:
				return false;
			}
		}

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
				"Dot", [](const Math::Vec3f& left, const Math::Vec3f& right) { return left | right; },
				"Cross", [](const Math::Vec3f& left, const Math::Vec3f& right) { return left ^ right; },
				"Length", [](const Math::Vec3f& v) { return Math::Sqrt(v | v); },
				"Normalized", [](const Math::Vec3f& v)
				{
					const f32 lengthSq = v | v;

					if (lengthSq < Math::Quat::SmallNumber)
						return Math::Vec3f::Zero();

					return v / Math::Sqrt(lengthSq);
				},
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
				"Identity", &Math::Quat::Identity,
				"FromAxisAngle", &Math::Quat::FromAxisAngle,
				"FromEuler", &Math::Quat::FromEuler,
				"ToEuler", &Math::Quat::ToEuler,
				"Normalized", &Math::Quat::GetNormalized,
				sol::meta_function::multiplication, sol::overload(
					sol::resolve<Math::Quat(const Math::Quat&) const>(&Math::Quat::operator*),
					sol::resolve<Math::Vec3f(const Math::Vec3f&) const>(&Math::Quat::operator*)));
		}

		void RegisterEntity(sol::state& lua, BindingState* pState)
		{
			lua.new_usertype<ComponentProxy>("Component",
				sol::no_constructor,
				sol::meta_function::index, [pState](sol::this_state ts, const ComponentProxy& proxy, const std::string& key) -> sol::object
				{
					sol::state_view view(ts);
					const Reflect::Type* pType = nullptr;
					ComponentObject* pComponent = ResolveComponent(ts, *pState, proxy, pType);
					const Reflect::Field* pField = FindReflectedField(*pState, *pType, key);

					if (pField == nullptr)
						luaL_error(ts, "%s has no field named %s", pType->GetName().c_str(), key.c_str());

					return ReadReflected(view, *pField, pComponent);
				},
				sol::meta_function::new_index, [pState](sol::this_state ts, const ComponentProxy& proxy, const std::string& key, const sol::object& value)
				{
					const Reflect::Type* pType = nullptr;
					ComponentObject* pComponent = ResolveComponent(ts, *pState, proxy, pType);
					const Reflect::Field* pField = FindReflectedField(*pState, *pType, key);

					if (pField == nullptr)
						luaL_error(ts, "%s has no field named %s", pType->GetName().c_str(), key.c_str());

					if (pField->GetCustomAttribute<Reflect::ReadOnlyAttribute>() != nullptr)
						luaL_error(ts, "%s.%s is read-only", pType->GetName().c_str(), key.c_str());

					if (!WriteReflected(*pField, pComponent, value))
						luaL_error(ts, "%s.%s cannot be assigned from a %s", pType->GetName().c_str(), key.c_str(), sol::type_name(ts, value.get_type()).c_str());
				});

			lua.new_usertype<EntityHandle>("EntityHandle",
				sol::no_constructor,
				"IsValid", &EntityHandle::IsValid,
				"Get", [pState](sol::this_state ts, const EntityHandle& entity, const std::string& typeName) -> sol::object
				{
					sol::state_view view(ts);

					if (pState->pScene == nullptr || !pState->pScene->GetEntities().IsAlive(entity))
						return sol::make_object(view, sol::lua_nil);

					const Reflect::Type* pType = pState->pReflection->GetTypeByName(typeName);

					if (pType == nullptr || pType->GetBaseId() != Reflect::TypeOf<ComponentObject>())
						luaL_error(ts, "%s is not a component type", typeName.c_str());

					if (!pState->pScene->HasComponent(entity, pType->GetTypeId()))
						return sol::make_object(view, sol::lua_nil);

					return sol::make_object(view, ComponentProxy{ entity, pType->GetTypeId() });
				},
				"Has", [pState](sol::this_state ts, const EntityHandle& entity, const std::string& typeName) -> bool
				{
					if (pState->pScene == nullptr || !pState->pScene->GetEntities().IsAlive(entity))
						return false;

					const Reflect::Type* pType = pState->pReflection->GetTypeByName(typeName);

					if (pType == nullptr || pType->GetBaseId() != Reflect::TypeOf<ComponentObject>())
						luaL_error(ts, "%s is not a component type", typeName.c_str());

					return pState->pScene->HasComponent(entity, pType->GetTypeId());
				},
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

	void ScriptContext::Reset()
	{
		DestroyState();
		CreateState();
	}

	void ScriptContext::BindWorld(Scene* pScene)
	{
		if (m_impl == nullptr)
			return;

		m_impl->bindings.pScene = pScene;
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
		m_impl->bindings.pReflection = GetEngine()->GetReflectionSystem();

		sol::state& lua = m_impl->lua;
		lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::string, sol::lib::table);

		RegisterConstructors(lua);
		RegisterMath(lua);
		RegisterEntity(lua, &m_impl->bindings);
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