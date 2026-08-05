#pragma once

#include <Runtime/Serialization/Archive.h>

#include <nlohmann/json.hpp>

#include <Runtime/Containers/List.h>

#include <string>

namespace Horizon
{
	class H_EXPORT JsonArchiveWriter final : public IArchiveWriter
	{
	public:
		void BeginObject() final;
		void EndObject() final;
		void Key(std::string_view name) final;
		void BeginArray(usize count) final;
		void EndArray() final;

		void WriteBool(b8 value) final;
		void WriteI64(i64 value) final;
		void WriteU64(u64 value) final;
		void WriteF64(f64 value) final;
		void WriteString(std::string_view value) final;

		std::string ToString() const;
		List<u8> ToBytes() const;

	private:
		nlohmann::json& NextSlot();

	private:
		nlohmann::json m_root;
		List<nlohmann::json*> m_stack;
		std::string m_pendingKey;
	};

	class H_EXPORT JsonArchiveReader final : public IArchiveReader
	{
	public:
		JsonArchiveReader(std::string_view text);

		void BeginObject() final;
		void EndObject() final;
		b8 Key(std::string_view name) final;
		usize BeginArray() final;
		void EndArray() final;

		b8 ReadBool() final;
		i64 ReadI64() final;
		u64 ReadU64() final;
		f64 ReadF64() final;
		std::string ReadString() final;

		b8 HasError() const final { return m_hasError; }

	private:
		struct Frame
		{
			const nlohmann::json* node = nullptr;
			usize readIndex = 0;
		};

		const nlohmann::json* Target();

	private:
		nlohmann::json m_root;
		List<Frame> m_stack;
		const nlohmann::json* m_current = nullptr;
		b8 m_hasError = false;
	};
}