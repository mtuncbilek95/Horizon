#pragma once

#include <Runtime/Serialization/Archive.h>

#include <nlohmann/json.hpp>

namespace Horizon
{
	class JsonArchive final : public Archive
	{
	public:
		JsonArchive();
		JsonArchive(const u8* data, usize size);

		b8 IsWriting() const final { return m_writing; }
		b8 IsValid() const { return m_valid; }

		std::vector<u8> ToBytes() const;

	protected:
		void OnBool(const std::string& key, b8& value) final { ReadWrite(key, value); }
		void OnI32(const std::string& key, i32& value) final { ReadWrite(key, value); }
		void OnI64(const std::string& key, i64& value) final { ReadWrite(key, value); }
		void OnU32(const std::string& key, u32& value) final { ReadWrite(key, value); }
		void OnU64(const std::string& key, u64& value) final { ReadWrite(key, value); }
		void OnF32(const std::string& key, f32& value) final { ReadWrite(key, value); }
		void OnF64(const std::string& key, f64& value) final { ReadWrite(key, value); }
		void OnString(const std::string& key, std::string& value) final { ReadWrite(key, value); }

		void BeginObject(const std::string& key) final;
		void EndObject() final;
		usize BeginArray(const std::string& key, usize count) final;
		void EndArray() final;

	private:
		struct Frame
		{
			nlohmann::json* node = nullptr;
			usize readIndex = 0;
		};

		template<typename T>
		void ReadWrite(const std::string& key, T& value)
		{
			if (m_writing)
			{
				WriteSlot(key) = value;
			}
			else
			{
				nlohmann::json* slot = FindSlot(key);
				if (slot)
				{
					value = slot->get<T>();
				}
			}
		}

		nlohmann::json& WriteSlot(const std::string& key);
		nlohmann::json* FindSlot(const std::string& key);

	private:
		nlohmann::json m_root;
		nlohmann::json m_missing = nlohmann::json::object();
		std::vector<Frame> m_stack;
		b8 m_writing = true;
		b8 m_valid = true;
	};
}