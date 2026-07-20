#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

#include <string>
#include <string_view>

namespace Horizon
{
	class H_EXPORT IArchiveWriter
	{
	public:
		virtual ~IArchiveWriter() = default;

		virtual void BeginObject() = 0;
		virtual void EndObject() = 0;
		virtual void Key(std::string_view name) = 0;

		virtual void BeginArray(usize count) = 0;
		virtual void EndArray() = 0;

		virtual void WriteBool(b8) = 0;
		virtual void WriteI64(i64) = 0;
		virtual void WriteU64(u64) = 0;
		virtual void WriteF64(f64) = 0;
		virtual void WriteString(std::string_view) = 0;
	};

	class H_EXPORT IArchiveReader
	{
	public:
		virtual ~IArchiveReader() = default;

		virtual void BeginObject() = 0;
		virtual void EndObject() = 0;
		virtual b8 Key(std::string_view name) = 0;
		virtual usize BeginArray() = 0;
		virtual void EndArray() = 0;

		virtual b8 ReadBool() = 0;
		virtual i64 ReadI64() = 0;
		virtual u64 ReadU64() = 0;
		virtual f64 ReadF64() = 0;
		virtual std::string ReadString() = 0;

		virtual b8 HasError() const = 0;
	};
}