#include "Serializer.h"

#include <Runtime/Containers/Guid.h>
#include <Runtime/Containers/ListBase.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/PAL/Timer/DateTime.h>
#include <Runtime/RTTR/Attributes/TransientAttribute.h>
#include <Runtime/RTTR/Attributes/AliasAttribute.h>

#include <string>

namespace Horizon
{
	namespace
	{
		void WriteScalar(const void* valuePtr, Reflect::TypeKind kind, IArchiveWriter& writer)
		{
			switch (kind)
			{
			case Reflect::TypeKind::Boolean:    writer.WriteBool(*static_cast<const b8*>(valuePtr)); break;
			case Reflect::TypeKind::Char:       writer.WriteI64(*static_cast<const c8*>(valuePtr)); break;
			case Reflect::TypeKind::Signed8:    writer.WriteI64(*static_cast<const i8*>(valuePtr)); break;
			case Reflect::TypeKind::Signed16:   writer.WriteI64(*static_cast<const i16*>(valuePtr)); break;
			case Reflect::TypeKind::Signed32:   writer.WriteI64(*static_cast<const i32*>(valuePtr)); break;
			case Reflect::TypeKind::Signed64:   writer.WriteI64(*static_cast<const i64*>(valuePtr)); break;
			case Reflect::TypeKind::Unsigned8:  writer.WriteU64(*static_cast<const u8*>(valuePtr)); break;
			case Reflect::TypeKind::Unsigned16: writer.WriteU64(*static_cast<const u16*>(valuePtr)); break;
			case Reflect::TypeKind::Unsigned32: writer.WriteU64(*static_cast<const u32*>(valuePtr)); break;
			case Reflect::TypeKind::Unsigned64: writer.WriteU64(*static_cast<const u64*>(valuePtr)); break;
			case Reflect::TypeKind::Float32:    writer.WriteF64(*static_cast<const f32*>(valuePtr)); break;
			case Reflect::TypeKind::Float64:    writer.WriteF64(*static_cast<const f64*>(valuePtr)); break;
			case Reflect::TypeKind::String:     writer.WriteString(*static_cast<const std::string*>(valuePtr)); break;
			default:
				Terminal::Error("Serializer", "Kind is not a scalar, nothing written");
				break;
			}
		}

		void ReadScalar(void* valuePtr, Reflect::TypeKind kind, IArchiveReader& reader)
		{
			switch (kind)
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
			default:
				Terminal::Error("Serializer", "Kind is not a scalar, nothing read");
				break;
			}
		}
	}

	void Serializer::Serialize(const void* pObject, const Reflect::Type& type, IArchiveWriter& writer)
	{
		WriteObject(pObject, type, writer);
	}

	void Serializer::WriteObject(const void* obj, const Reflect::Type& type, IArchiveWriter& writer)
	{
		writer.BeginObject();

		for (const Reflect::Field& field : type.GetFields())
		{
			if (field.GetCustomAttribute<Reflect::TransientAttribute>())
				continue;

			writer.Key(field.GetName());
			WriteField(field.GetValue(obj), field, writer);
		}

		writer.EndObject();
	}

	void Serializer::WriteField(const void* valuePtr, const Reflect::Field& field, IArchiveWriter& writer)
	{
		if (field.GetMode() == Reflect::TypeMode::Pointer)
		{
			WritePointer(valuePtr, field, writer);
			return;
		}

		if (field.GetMode() == Reflect::TypeMode::Array)
		{
			const ListBase* pList = static_cast<const ListBase*>(valuePtr);
			const usize count = pList->GetCount();

			writer.BeginArray(count);

			for (usize i = 0; i < count; ++i)
				WriteValue(pList->GetElementAt(i), field, writer);

			writer.EndArray();
			return;
		}

		WriteValue(valuePtr, field, writer);
	}

	void Serializer::WritePointer(const void* pointerSlot, const Reflect::Field& field, IArchiveWriter& writer)
	{
		const Reflect::Base* pTarget = *static_cast<const Reflect::Base* const*>(pointerSlot);

		writer.BeginObject();

		if (pTarget)
		{
			const Reflect::Type* pType = Resolve(pTarget->GetTypeId());
			if (pType)
			{
				writer.Key("type");
				writer.WriteString(pType->GetName());

				writer.Key("data");
				WriteObject(pTarget, *pType, writer);
			}
			else
			{
				Terminal::Warn("Serializer", "Unregistered concrete type in field '{}'", field.GetName());
			}
		}

		writer.EndObject();
	}

	void Serializer::WriteValue(const void* valuePtr, const Reflect::Field& field, IArchiveWriter& writer)
	{
		const Reflect::TypeKind kind = field.GetKind();

		if (kind == Reflect::TypeKind::Enum)
		{
			WriteScalar(valuePtr, field.GetUnderlyingKind(), writer);
			return;
		}

		if (kind != Reflect::TypeKind::Object)
		{
			WriteScalar(valuePtr, kind, writer);
			return;
		}

		if (field.GetTypeId() == Reflect::TypeOf<Guid>())
		{
			writer.WriteString(static_cast<const Guid*>(valuePtr)->ToString());
			return;
		}

		if (field.GetTypeId() == Reflect::TypeOf<PAL::DateTime>())
		{
			writer.WriteString(static_cast<const PAL::DateTime*>(valuePtr)->ToString());
			return;
		}

		const Reflect::Type* nested = Resolve(field.GetTypeId());
		if (!nested)
		{
			Terminal::Warn("Serializer", "Cannot resolve nested type for field '{}'", field.GetName());
			writer.BeginObject();
			writer.EndObject();
			return;
		}

		WriteObject(valuePtr, *nested, writer);
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
			if (field.GetCustomAttribute<Reflect::TransientAttribute>())
				continue;

			if (SeekField(field, reader))
				ReadField(field.GetValue(obj), field, reader);
		}

		reader.EndObject();
	}

	void Serializer::ReadField(void* valuePtr, const Reflect::Field& field, IArchiveReader& reader)
	{
		if (field.GetMode() == Reflect::TypeMode::Pointer)
		{
			ReadPointer(valuePtr, field, reader);
			return;
		}

		if (field.GetMode() == Reflect::TypeMode::Array)
		{
			ListBase* pList = static_cast<ListBase*>(valuePtr);
			const usize count = reader.BeginArray();

			pList->Resize(count);

			for (usize i = 0; i < count; ++i)
				ReadValue(pList->GetElementAt(i), field, reader);

			reader.EndArray();
			return;
		}

		ReadValue(valuePtr, field, reader);
	}

	void Serializer::ReadPointer(void* pointerSlot, const Reflect::Field& field, IArchiveReader& reader)
	{
		Reflect::Base* pTarget = *static_cast<Reflect::Base**>(pointerSlot);

		reader.BeginObject();

		if (reader.Key("type"))
		{
			const std::string typeName = reader.ReadString();
			const Reflect::Type* pType = pTarget ? Resolve(pTarget->GetTypeId()) : nullptr;

			if (!pTarget)
				Terminal::Warn("Serializer", "Field '{}' holds no instance, '{}' data skipped", field.GetName(), typeName);
			else if (!pType)
				Terminal::Warn("Serializer", "Unregistered concrete type in field '{}'", field.GetName());
			else if (pType->GetName() != typeName)
				Terminal::Warn("Serializer", "Field '{}' holds '{}' but archive has '{}', skipped", field.GetName(), pType->GetName(), typeName);
			else if (reader.Key("data"))
				ReadObject(pTarget, *pType, reader);
		}

		reader.EndObject();
	}

	b8 Serializer::SeekField(const Reflect::Field& field, IArchiveReader& reader)
	{
		if (reader.Key(field.GetName()))
			return true;

		for (const Reflect::AliasAttribute* pAlias : field.GetCustomAttributes<Reflect::AliasAttribute>())
		{
			if (reader.Key(pAlias->GetFormerName()))
				return true;
		}

		return false;
	}

	void Serializer::ReadValue(void* valuePtr, const Reflect::Field& field, IArchiveReader& reader)
	{
		const Reflect::TypeKind kind = field.GetKind();

		if (kind == Reflect::TypeKind::Enum)
		{
			ReadScalar(valuePtr, field.GetUnderlyingKind(), reader);
			return;
		}

		if (kind != Reflect::TypeKind::Object)
		{
			ReadScalar(valuePtr, kind, reader);
			return;
		}

		if (field.GetTypeId() == Reflect::TypeOf<Guid>())
		{
			*static_cast<Guid*>(valuePtr) = Guid(reader.ReadString());
			return;
		}

		if (field.GetTypeId() == Reflect::TypeOf<PAL::DateTime>())
		{
			*static_cast<PAL::DateTime*>(valuePtr) = PAL::DateTime::FromStringToDateTime(reader.ReadString());
			return;
		}

		const Reflect::Type* nested = Resolve(field.GetTypeId());
		if (!nested)
		{
			Terminal::Error("Serializer", "Previous error was related with {}.", field.GetName());
			return;
		}

		ReadObject(valuePtr, *nested, reader);
	}
}