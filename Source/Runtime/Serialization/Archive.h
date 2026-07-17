#pragma once

#include <Runtime/Reflection/TypeManifest.h>
#include <Runtime/Reflection/FieldManifest.h>

#include <string>
#include <vector>

namespace Horizon
{
	class Archive
	{
	public:
		virtual ~Archive() = default;

		virtual b8 IsWriting() const = 0;

		void Serialize(void* pObj, TypeManifest* pManifest)
		{
			for (const FieldManifest& field : pManifest->GetFields())
				field.serialize(pObj, *this);
		}

		void Field(const std::string& key, b8& value) { OnBool(key, value); }
		void Field(const std::string& key, i32& value) { OnI32(key, value); }
		void Field(const std::string& key, i64& value) { OnI64(key, value); }
		void Field(const std::string& key, u32& value) { OnU32(key, value); }
		void Field(const std::string& key, u64& value) { OnU64(key, value); }
		void Field(const std::string& key, f32& value) { OnF32(key, value); }
		void Field(const std::string& key, f64& value) { OnF64(key, value); }
		void Field(const std::string& key, std::string& value) { OnString(key, value); }

		template<typename T>
		void Field(const std::string& key, std::vector<T>& value)
		{
			usize count = BeginArray(key, value.size());

			if (!IsWriting())
				value.resize(count);

			for (usize i = 0; i < count; ++i)
				Field(std::string(), value[i]);

			EndArray();
		}

	protected:
		virtual void OnBool(const std::string& key, b8& value) = 0;
		virtual void OnI32(const std::string& key, i32& value) = 0;
		virtual void OnI64(const std::string& key, i64& value) = 0;
		virtual void OnU32(const std::string& key, u32& value) = 0;
		virtual void OnU64(const std::string& key, u64& value) = 0;
		virtual void OnF32(const std::string& key, f32& value) = 0;
		virtual void OnF64(const std::string& key, f64& value) = 0;
		virtual void OnString(const std::string& key, std::string& value) = 0;

		virtual void BeginObject(const std::string& key) = 0;
		virtual void EndObject() = 0;
		virtual usize BeginArray(const std::string& key, usize count) = 0;
		virtual void EndArray() = 0;
	};
}