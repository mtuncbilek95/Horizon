#pragma once

#include <Runtime/Containers/Guid.h>
#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/RTTR/Reflection.h>
#include <Runtime/RTTR/TypeHandle.h>
#include <Runtime/Math/Quat.h>
#include <Runtime/Math/Vec3f.h>
#include <Runtime/Math/Vec2f.h>

#include <string>

namespace Horizon::Engine
{
	HENUM();
	enum class ScriptFieldKind : u8
	{
		Bool,
		Integer,
		Number,
		String,
		Vector2,
		Vector3,
		Quaternion,
		Entity,
		Asset
	};

	// Default data capable fields
	HCLASS();
	struct ScriptFieldValue
	{
		HORIZON_DATA_REFLECT(ScriptFieldValue);

		HFIELD();
		b8 boolValue = false;

		HFIELD();
		i64 integerValue = 0;

		HFIELD();
		f64 numberValue = 0.0;

		HFIELD();
		std::string stringValue;

		HFIELD();
		Math::Quat quatValue;

		HFIELD();
		Math::Vec3f vec3Value;

		HFIELD();
		Math::Vec2f vec2Value;

		HFIELD();
		Guid assetValue;
	};

	struct ScriptField
	{
		std::string name;
		ScriptFieldKind kind = ScriptFieldKind::Number;
		ScriptFieldValue defaultValue;

		std::string assetTypeName;
		Reflect::TypeHandle assetType;

		b8 hasRange = false;
		f64 min = 0.0;
		f64 max = 0.0;

		b8 readOnly = false;
	};
}