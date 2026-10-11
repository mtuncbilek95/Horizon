#include "ScriptSchema.h"

#include <Engine/Asset/AssetObject.h>
#include <Engine/Reflection/ReflectionSystem.h>
#include <Runtime/Log/Terminal.h>

#include <sol/sol.hpp>

#include <format>

namespace Horizon::Engine
{
	namespace
	{
		void InstructionLimitHook(lua_State* pState, lua_Debug* pDebug)
		{
			luaL_error(pState, "Script exceeded the instruction limit while declaring its properties");
		}

		sol::table MakeMarker(sol::this_state state, const char* kind)
		{
			sol::state_view view(state);
			sol::table marker = view.create_table();
			marker["__kind"] = kind;
			return marker;
		}

		void RegisterConstructors(sol::state& lua)
		{
			lua.set_function("Asset", [](sol::this_state state, const std::string& typeName)
				{
					sol::table marker = MakeMarker(state, "Asset");
					marker["type"] = typeName;
					return marker;
				});

			lua.set_function("Entity", [](sol::this_state state)
				{
					return MakeMarker(state, "Entity");
				});

			lua.set_function("Quaternion", [](sol::this_state state, sol::optional<f64> x, sol::optional<f64> y, sol::optional<f64> z, sol::optional<f64> w)
				{
					sol::table marker = MakeMarker(state, "Quaternion");
					marker["x"] = x.value_or(0.0);
					marker["y"] = y.value_or(0.0);
					marker["z"] = z.value_or(0.0);
					marker["w"] = w.value_or(1.0);
					return marker;
				});

			lua.set_function("Vector2", [](sol::this_state state, sol::optional<f64> x, sol::optional<f64> y)
				{
					sol::table marker = MakeMarker(state, "Vector2");
					marker["x"] = x.value_or(0.0);
					marker["y"] = y.value_or(0.0);
					return marker;
				});

			lua.set_function("Vector3", [](sol::this_state state, sol::optional<f64> x, sol::optional<f64> y, sol::optional<f64> z)
				{
					sol::table marker = MakeMarker(state, "Vector3");
					marker["x"] = x.value_or(0.0);
					marker["y"] = y.value_or(0.0);
					marker["z"] = z.value_or(0.0);
					return marker;
				});

			lua.set_function("Range", [](sol::this_state state, f64 value, f64 min, f64 max)
				{
					sol::table marker = MakeMarker(state, "Range");
					marker["value"] = value;
					marker["min"] = min;
					marker["max"] = max;
					return marker;
				});

			lua.set_function("ReadOnly", [](sol::this_state state, sol::object value)
				{
					sol::table marker = MakeMarker(state, "ReadOnly");
					marker["value"] = value;
					return marker;
				});
		}

		b8 IsInteger(const sol::object& value)
		{
			lua_State* pState = value.lua_state();

			value.push();
			const b8 isInteger = lua_isinteger(pState, -1) != 0;
			lua_pop(pState, 1);

			return isInteger;
		}

		template<typename T>
		T ValueOr(const sol::table& table, const char* key, T fallback)
		{
			return table.get<sol::optional<T>>(key).value_or(fallback);
		}

		b8 ReadMarker(const sol::table& marker, ReflectionSystem* pReflection, ScriptField& outField, std::string& outError)
		{
			const std::string kind = ValueOr<std::string>(marker, "__kind", std::string());

			if (kind == "Entity")
			{
				outField.kind = ScriptFieldKind::Entity;
				return true;
			}

			if (kind == "Vector2")
			{
				outField.kind = ScriptFieldKind::Vector2;
				outField.defaultValue.vec2Value = Math::Vec2f(ValueOr<f32>(marker, "x", 0.0f), ValueOr<f32>(marker, "y", 0.0f));
				return true;
			}

			if (kind == "Vector3")
			{
				outField.kind = ScriptFieldKind::Vector3;
				outField.defaultValue.vec3Value = Math::Vec3f(ValueOr<f32>(marker, "x", 0.0f), ValueOr<f32>(marker, "y", 0.0f), ValueOr<f32>(marker, "z", 0.0f));
				return true;
			}

			if (kind == "Quaternion")
			{
				outField.kind = ScriptFieldKind::Quaternion;
				outField.defaultValue.quatValue = Math::Quat(ValueOr<f32>(marker, "x", 0.0f), ValueOr<f32>(marker, "y", 0.0f), ValueOr<f32>(marker, "z", 0.0f), ValueOr<f32>(marker, "w", 1.0f));
				return true;
			}

			if (kind == "Range")
			{
				outField.kind = ScriptFieldKind::Number;
				outField.defaultValue.numberValue = ValueOr<f64>(marker, "value", 0.0);
				outField.hasRange = true;
				outField.min = ValueOr<f64>(marker, "min", 0.0);
				outField.max = ValueOr<f64>(marker, "max", 0.0);
				return true;
			}

			if (kind == "Asset")
			{
				outField.kind = ScriptFieldKind::Asset;
				outField.assetTypeName = ValueOr<std::string>(marker, "type", std::string());

				const Reflect::Type* pType = pReflection->GetTypeByName(outField.assetTypeName);

				if (pType == nullptr || pType->GetBaseId() != Reflect::TypeOf<AssetObject>())
				{
					outError = std::format("{} is not an asset type", outField.assetTypeName);
					return false;
				}

				outField.assetType = pType->GetTypeId();
				return true;
			}

			outError = "plain tables are not supported, use Range, Vector2, Vector3, Quaternion, Entity or Asset";
			return false;
		}

		b8 ReadField(const std::string& name, const sol::object& value, ReflectionSystem* pReflection, ScriptField& outField, std::string& outError)
		{
			// Just like a fuckin json, isn't it?

			outField.name = name;

			switch (value.get_type())
			{
			case sol::type::boolean:
				outField.kind = ScriptFieldKind::Bool;
				outField.defaultValue.boolValue = value.as<bool>();
				return true;
			case sol::type::number:
				if (IsInteger(value))
				{
					outField.kind = ScriptFieldKind::Integer;
					outField.defaultValue.integerValue = value.as<i64>();
					return true;
				}

				outField.kind = ScriptFieldKind::Number;
				outField.defaultValue.numberValue = value.as<f64>();
				return true;
			case sol::type::string:
				outField.kind = ScriptFieldKind::String;
				outField.defaultValue.stringValue = value.as<std::string>();
				return true;
			case sol::type::table:
			{
				const sol::table marker = value.as<sol::table>();

				if (ValueOr<std::string>(marker, "__kind", std::string()) == "ReadOnly")
				{
					outField.readOnly = true;
					return ReadField(name, marker.get<sol::object>("value"), pReflection, outField, outError);
				}

				return ReadMarker(marker, pReflection, outField, outError);
			}
			default:
				outError = "unsupported value type";
				return false;
			}
		}
	}

	b8 ScriptSchema::Extract(std::string_view source, const std::string& chunkName, ReflectionSystem* pReflection, List<ScriptField>& outFields, std::string& outError)
	{
		// Here we go!
		sol::state lua;
		lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::string, sol::lib::table);
		RegisterConstructors(lua);
		lua_sethook(lua.lua_state(), InstructionLimitHook, LUA_MASKCOUNT, MaxInstructions);

		sol::protected_function_result result = lua.safe_script(source, sol::script_pass_on_error, chunkName, sol::load_mode::text);

		if (!result.valid())
		{
			sol::error error = result;
			outError = error.what();
			return false;
		}

		sol::object returned = result.get<sol::object>();

		if (returned.get_type() != sol::type::table)
		{
			outError = std::format("{} must return its class table", chunkName);
			return false;
		}

		sol::object properties = returned.as<sol::table>().get<sol::object>("Properties");

		if (properties.get_type() == sol::type::lua_nil)
			return true;

		if (properties.get_type() != sol::type::table)
		{
			outError = std::format("{} Properties must be a table", chunkName);
			return false;
		}

		for (const auto& [key, value] : properties.as<sol::table>())
		{
			if (key.get_type() != sol::type::string)
				continue;

			const std::string name = key.as<std::string>();
			ScriptField field;
			std::string fieldError;

			if (!ReadField(name, value, pReflection, field, fieldError))
			{
				Terminal::Warn("ScriptSchema", "{} field {} skipped, {}", chunkName, name, fieldError);
				continue;
			}

			outFields.PushBack(std::move(field));
		}

		outFields.Sort([](const ScriptField& left, const ScriptField& right)
			{
				return left.name < right.name;
			});

		return true;
	}
}