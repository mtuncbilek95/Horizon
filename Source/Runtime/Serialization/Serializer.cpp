#include "Serializer.h"

#include <Runtime/Containers/Guid.h>
#include <Runtime/Containers/ListBase.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/PAL/Timer/DateTime.h>
#include <Runtime/RTTR/Base.h>
#include <Runtime/RTTR/Attributes/TransientAttribute.h>
#include <Runtime/RTTR/Attributes/AliasAttribute.h>

#include <string>

namespace Horizon
{
	namespace
	{
		void WriteScalar(const void* pValue, Reflect::TypeKind kind, IArchiveWriter& writer)
		{
			switch (kind)
			{
			case Reflect::TypeKind::Boolean:    writer.WriteBool(*static_cast<const b8*>(pValue)); break;
			case Reflect::TypeKind::Char:       writer.WriteI64(*static_cast<const c8*>(pValue)); break;
			case Reflect::TypeKind::Signed8:    writer.WriteI64(*static_cast<const i8*>(pValue)); break;
			case Reflect::TypeKind::Signed16:   writer.WriteI64(*static_cast<const i16*>(pValue)); break;
			case Reflect::TypeKind::Signed32:   writer.WriteI64(*static_cast<const i32*>(pValue)); break;
			case Reflect::TypeKind::Signed64:   writer.WriteI64(*static_cast<const i64*>(pValue)); break;
			case Reflect::TypeKind::Unsigned8:  writer.WriteU64(*static_cast<const u8*>(pValue)); break;
			case Reflect::TypeKind::Unsigned16: writer.WriteU64(*static_cast<const u16*>(pValue)); break;
			case Reflect::TypeKind::Unsigned32: writer.WriteU64(*static_cast<const u32*>(pValue)); break;
			case Reflect::TypeKind::Unsigned64: writer.WriteU64(*static_cast<const u64*>(pValue)); break;
			case Reflect::TypeKind::Float32:    writer.WriteF64(*static_cast<const f32*>(pValue)); break;
			case Reflect::TypeKind::Float64:    writer.WriteF64(*static_cast<const f64*>(pValue)); break;
			case Reflect::TypeKind::String:     writer.WriteString(*static_cast<const std::string*>(pValue)); break;
			default:
				Terminal::Error("Serializer", "Kind is not a scalar, nothing written");
				break;
			}
		}

		void ReadScalar(void* pValue, Reflect::TypeKind kind, IArchiveReader& reader)
		{
			switch (kind)
			{
			case Reflect::TypeKind::Boolean:    *static_cast<b8*>(pValue) = reader.ReadBool(); break;
			case Reflect::TypeKind::Char:       *static_cast<c8*>(pValue) = static_cast<c8>(reader.ReadI64()); break;
			case Reflect::TypeKind::Signed8:    *static_cast<i8*>(pValue) = static_cast<i8>(reader.ReadI64()); break;
			case Reflect::TypeKind::Signed16:   *static_cast<i16*>(pValue) = static_cast<i16>(reader.ReadI64()); break;
			case Reflect::TypeKind::Signed32:   *static_cast<i32*>(pValue) = static_cast<i32>(reader.ReadI64()); break;
			case Reflect::TypeKind::Signed64:   *static_cast<i64*>(pValue) = reader.ReadI64(); break;
			case Reflect::TypeKind::Unsigned8:  *static_cast<u8*>(pValue) = static_cast<u8>(reader.ReadU64()); break;
			case Reflect::TypeKind::Unsigned16: *static_cast<u16*>(pValue) = static_cast<u16>(reader.ReadU64()); break;
			case Reflect::TypeKind::Unsigned32: *static_cast<u32*>(pValue) = static_cast<u32>(reader.ReadU64()); break;
			case Reflect::TypeKind::Unsigned64: *static_cast<u64*>(pValue) = reader.ReadU64(); break;
			case Reflect::TypeKind::Float32:    *static_cast<f32*>(pValue) = static_cast<f32>(reader.ReadF64()); break;
			case Reflect::TypeKind::Float64:    *static_cast<f64*>(pValue) = reader.ReadF64(); break;
			case Reflect::TypeKind::String:     *static_cast<std::string*>(pValue) = reader.ReadString(); break;
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

	void Serializer::WriteObject(const void* pObject, const Reflect::Type& type, IArchiveWriter& writer)
	{
		writer.BeginObject();
		WriteFields(pObject, type, writer);
		writer.EndObject();
	}

	void Serializer::WriteFields(const void* pObject, const Reflect::Type& type, IArchiveWriter& writer)
	{
		const Reflect::Type* pBase = ResolveBase(type);

		if (pBase)
			WriteFields(static_cast<const c8*>(pObject) + type.GetBaseOffset(), *pBase, writer);

		for (const Reflect::Field& field : type.GetFields())
		{
			if (field.GetCustomAttribute<Reflect::TransientAttribute>())
				continue;

			writer.Key(field.GetName());
			WriteField(field.GetValue(pObject), field, writer);
		}
	}

	void Serializer::WriteField(const void* pValue, const Reflect::Field& field, IArchiveWriter& writer)
	{
		if (field.GetMode() == Reflect::TypeMode::Pointer)
		{
			WritePointer(pValue, field, writer);
			return;
		}

		if (field.GetMode() == Reflect::TypeMode::Array)
		{
			const ListBase* pList = static_cast<const ListBase*>(pValue);
			const usize count = pList->GetCount();

			writer.BeginArray(count);

			for (usize i = 0; i < count; ++i)
				WriteValue(pList->GetElementAt(i), field, writer);

			writer.EndArray();
			return;
		}

		WriteValue(pValue, field, writer);
	}

	void Serializer::WritePointer(const void* pPointerSlot, const Reflect::Field& field, IArchiveWriter& writer)
	{
		const Reflect::Base* pTarget = *static_cast<const Reflect::Base* const*>(pPointerSlot);

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

	void Serializer::WriteValue(const void* pValue, const Reflect::Field& field, IArchiveWriter& writer)
	{
		const Reflect::TypeKind kind = field.GetKind();

		if (kind == Reflect::TypeKind::Enum)
		{
			WriteScalar(pValue, field.GetUnderlyingKind(), writer);
			return;
		}

		if (kind != Reflect::TypeKind::Object)
		{
			WriteScalar(pValue, kind, writer);
			return;
		}

		if (field.GetTypeId() == Reflect::TypeOf<Guid>())
		{
			writer.WriteString(static_cast<const Guid*>(pValue)->ToString());
			return;
		}

		if (field.GetTypeId() == Reflect::TypeOf<PAL::DateTime>())
		{
			writer.WriteString(static_cast<const PAL::DateTime*>(pValue)->ToString());
			return;
		}

		const Reflect::Type* pNested = Resolve(field.GetTypeId());
		if (!pNested)
		{
			Terminal::Warn("Serializer", "Cannot resolve nested type for field '{}'", field.GetName());
			writer.BeginObject();
			writer.EndObject();
			return;
		}

		WriteObject(pValue, *pNested, writer);
	}

	void Serializer::Deserialize(void* pObject, const Reflect::Type& type, IArchiveReader& reader)
	{
		ReadObject(pObject, type, reader);
	}

	void Serializer::ReadObject(void* pObject, const Reflect::Type& type, IArchiveReader& reader)
	{
		reader.BeginObject();
		ReadFields(pObject, type, reader);
		reader.EndObject();
	}

	void Serializer::ReadFields(void* pObject, const Reflect::Type& type, IArchiveReader& reader)
	{
		const Reflect::Type* pBase = ResolveBase(type);

		if (pBase)
			ReadFields(static_cast<c8*>(pObject) + type.GetBaseOffset(), *pBase, reader);

		for (const Reflect::Field& field : type.GetFields())
		{
			if (field.GetCustomAttribute<Reflect::TransientAttribute>())
				continue;

			if (SeekField(field, reader))
				ReadField(field.GetValue(pObject), field, reader);
		}
	}

	void Serializer::ReadField(void* pValue, const Reflect::Field& field, IArchiveReader& reader)
	{
		if (field.GetMode() == Reflect::TypeMode::Pointer)
		{
			ReadPointer(pValue, field, reader);
			return;
		}

		if (field.GetMode() == Reflect::TypeMode::Array)
		{
			ListBase* pList = static_cast<ListBase*>(pValue);
			const usize count = reader.BeginArray();

			pList->Resize(count);

			for (usize i = 0; i < count; ++i)
				ReadValue(pList->GetElementAt(i), field, reader);

			reader.EndArray();
			return;
		}

		ReadValue(pValue, field, reader);
	}

	void Serializer::ReadPointer(void* pPointerSlot, const Reflect::Field& field, IArchiveReader& reader)
	{
		Reflect::Base* pTarget = *static_cast<Reflect::Base**>(pPointerSlot);

		reader.BeginObject();

		if (reader.Key("type"))
		{
			const std::string typeName = reader.ReadString();
			const Reflect::Type* pType = nullptr;

			if (pTarget)
				pType = Resolve(pTarget->GetTypeId());
			else
			{
				pType = ResolveByName(typeName);

				if (pType && pType->CanConstruct())
				{
					pTarget = static_cast<Reflect::Base*>(pType->Create());
					*static_cast<Reflect::Base**>(pPointerSlot) = pTarget;
				}
			}

			if (!pTarget)
				Terminal::Warn("Serializer", "Field '{}' holds no instance and '{}' cannot be created, data skipped", field.GetName(), typeName);
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

	const Reflect::Type* Serializer::ResolveBase(const Reflect::Type& type)
	{
		const Reflect::TypeHandle baseId = type.GetBaseId();

		if (!baseId.IsValid() || baseId == Reflect::TypeOf<Reflect::Base>())
			return nullptr;

		const Reflect::Type* pBase = Resolve(baseId);

		if (!pBase)
		{
			Terminal::Warn("Serializer", "Base of '{}' is not registered, inherited fields skipped", type.GetName());
			return nullptr;
		}

		return pBase;
	}

	void Serializer::ReadValue(void* pValue, const Reflect::Field& field, IArchiveReader& reader)
	{
		const Reflect::TypeKind kind = field.GetKind();

		if (kind == Reflect::TypeKind::Enum)
		{
			ReadScalar(pValue, field.GetUnderlyingKind(), reader);
			return;
		}

		if (kind != Reflect::TypeKind::Object)
		{
			ReadScalar(pValue, kind, reader);
			return;
		}

		if (field.GetTypeId() == Reflect::TypeOf<Guid>())
		{
			*static_cast<Guid*>(pValue) = Guid(reader.ReadString());
			return;
		}

		if (field.GetTypeId() == Reflect::TypeOf<PAL::DateTime>())
		{
			*static_cast<PAL::DateTime*>(pValue) = PAL::DateTime::FromStringToDateTime(reader.ReadString());
			return;
		}

		const Reflect::Type* pNested = Resolve(field.GetTypeId());
		if (!pNested)
		{
			Terminal::Error("Serializer", "Previous error was related with {}.", field.GetName());
			return;
		}

		ReadObject(pValue, *pNested, reader);
	}
}