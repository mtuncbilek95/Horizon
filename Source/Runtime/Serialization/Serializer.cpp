#include "Serializer.h"

#include <Runtime/Containers/Guid.h>
#include <Runtime/Containers/ListBase.h>
#include <Runtime/Log/Terminal.h>

#include <string>

namespace Horizon
{
	void Serializer::Serialize(const void* pObject, const Reflect::Type& type, IArchiveWriter& writer)
	{
		WriteObject(pObject, type, writer);
	}

	void Serializer::WriteObject(const void* obj, const Reflect::Type& type, IArchiveWriter& writer)
	{
		writer.BeginObject();

		for (const Reflect::Field& field : type.GetFields())
		{
			writer.Key(field.GetName());
			WriteField(field.GetValue(obj), field, writer);
		}

		writer.EndObject();
	}

	void Serializer::WriteField(const void* valuePtr, const Reflect::Field& field, IArchiveWriter& writer)
	{
		if (field.GetMode() == Reflect::TypeMode::Array)
		{
			const ListBase* pList = static_cast<const ListBase*>(valuePtr);
			const usize count = pList->GetCount();

			writer.BeginArray(count);

			for (usize i = 0; i < count; ++i)
				WriteValue(pList->GetElement(i, field.GetElementSize()), field, writer);

			writer.EndArray();
			return;
		}

		WriteValue(valuePtr, field, writer);
	}

	void Serializer::WriteValue(const void* valuePtr, const Reflect::Field& field, IArchiveWriter& writer)
	{
		switch (field.GetKind())
		{
		case Reflect::TypeKind::Boolean:
			writer.WriteBool(*static_cast<const b8*>(valuePtr));
			break;
		case Reflect::TypeKind::Char:
			writer.WriteI64(*static_cast<const c8*>(valuePtr));
			break;
		case Reflect::TypeKind::Signed8:
			writer.WriteI64(*static_cast<const i8*>(valuePtr));
			break;
		case Reflect::TypeKind::Signed16:
			writer.WriteI64(*static_cast<const i16*>(valuePtr));
			break;
		case Reflect::TypeKind::Signed32:
			writer.WriteI64(*static_cast<const i32*>(valuePtr));
			break;
		case Reflect::TypeKind::Signed64:
			writer.WriteI64(*static_cast<const i64*>(valuePtr));
			break;
		case Reflect::TypeKind::Unsigned8:
			writer.WriteU64(*static_cast<const u8*>(valuePtr));
			break;
		case Reflect::TypeKind::Unsigned16:
			writer.WriteU64(*static_cast<const u16*>(valuePtr));
			break;
		case Reflect::TypeKind::Unsigned32:
			writer.WriteU64(*static_cast<const u32*>(valuePtr));
			break;
		case Reflect::TypeKind::Unsigned64:
			writer.WriteU64(*static_cast<const u64*>(valuePtr));
			break;
		case Reflect::TypeKind::Float32:
			writer.WriteF64(*static_cast<const f32*>(valuePtr));
			break;
		case Reflect::TypeKind::Float64:
			writer.WriteF64(*static_cast<const f64*>(valuePtr));
			break;
		case Reflect::TypeKind::String:
			writer.WriteString(*static_cast<const std::string*>(valuePtr));
			break;

		case Reflect::TypeKind::Enum:
			writer.WriteI64(*static_cast<const i32*>(valuePtr));
			break;

		case Reflect::TypeKind::Object:
		{
			if (field.GetTypeId() == Reflect::TypeOf<Guid>())
			{
				writer.WriteString(static_cast<const Guid*>(valuePtr)->ToString());
				break;
			}

			const Reflect::Type* nested = Resolve(field.GetTypeId());
			if (!nested)
			{
				Terminal::Warn("Serializer", "Cannot resolve nested type for field '{}'", field.GetName());
				writer.BeginObject();
				writer.EndObject();
				break;
			}

			WriteObject(valuePtr, *nested, writer);
			break;
		}
		}
	}

	void Serializer::Deserialize(void* pObject, const Reflect::Type& type, IArchiveReader& reader)
	{
		ReadObject(pObject, type, reader);
	}

	void Serializer::ReadObject(void* obj, const Reflect::Type& type, IArchiveReader& reader)
	{
		reader.BeginObject();

		for (const Reflect::Field& field : type.GetFields())
		{
			if (reader.Key(field.GetName()))
				ReadField(field.GetValue(obj), field, reader);
		}

		reader.EndObject();
	}

	void Serializer::ReadField(void* valuePtr, const Reflect::Field& field, IArchiveReader& reader)
	{
		if (field.GetMode() == Reflect::TypeMode::Array)
		{
			ListBase* pList = static_cast<ListBase*>(valuePtr);
			const usize count = reader.BeginArray();

			pList->ResizeErased(count, field.GetElementSize(), field.GetElementOps());

			for (usize i = 0; i < count; ++i)
				ReadValue(pList->GetElement(i, field.GetElementSize()), field, reader);

			reader.EndArray();
			return;
		}

		ReadValue(valuePtr, field, reader);
	}

	void Serializer::ReadValue(void* valuePtr, const Reflect::Field& field, IArchiveReader& reader)
	{
		switch (field.GetKind())
		{
		case Reflect::TypeKind::Boolean:    *static_cast<b8*>(valuePtr) = reader.ReadBool(); break;
		case Reflect::TypeKind::Char:       *static_cast<c8*>(valuePtr) = static_cast<c8>(reader.ReadI64()); break;
		case Reflect::TypeKind::Signed8:    *static_cast<i8*>(valuePtr) = static_cast<i8>(reader.ReadI64()); break;
		case Reflect::TypeKind::Signed16:   *static_cast<i16*>(valuePtr) = static_cast<i16>(reader.ReadI64()); break;
		case Reflect::TypeKind::Signed32:   *static_cast<i32*>(valuePtr) = static_cast<i32>(reader.ReadI64()); break;
		case Reflect::TypeKind::Signed64:   *static_cast<i64*>(valuePtr) = reader.ReadI64(); break;
		case Reflect::TypeKind::Unsigned8:  *static_cast<u8*>(valuePtr) = static_cast<u8>(reader.ReadU64()); break;
		case Reflect::TypeKind::Unsigned16: *static_cast<u16*>(valuePtr) = static_cast<u16>(reader.ReadU64()); break;
		case Reflect::TypeKind::Unsigned32: *static_cast<u32*>(valuePtr) = static_cast<u32>(reader.ReadU64()); break;
		case Reflect::TypeKind::Unsigned64: *static_cast<u64*>(valuePtr) = reader.ReadU64(); break;
		case Reflect::TypeKind::Float32:    *static_cast<f32*>(valuePtr) = static_cast<f32>(reader.ReadF64()); break;
		case Reflect::TypeKind::Float64:    *static_cast<f64*>(valuePtr) = reader.ReadF64(); break;
		case Reflect::TypeKind::String:     *static_cast<std::string*>(valuePtr) = reader.ReadString(); break;

		case Reflect::TypeKind::Enum:
			*static_cast<i32*>(valuePtr) = static_cast<i32>(reader.ReadI64());
			break;

		case Reflect::TypeKind::Object:
		{
			if (field.GetTypeId() == Reflect::TypeOf<Guid>())
			{
				*static_cast<Guid*>(valuePtr) = Guid(reader.ReadString());
				break;
			}

			const Reflect::Type* nested = Resolve(field.GetTypeId());
			if (nested)
				ReadObject(valuePtr, *nested, reader);

			break;
		}
		}
	}
}