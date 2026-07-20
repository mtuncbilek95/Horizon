#pragma once

#include <Runtime/Serialization/Archive.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon
{
	class H_EXPORT Serializer
	{
	public:
		using ResolveFn = const Reflect::Type* (*)(void* userData, Reflect::TypeHandle handle);

		Serializer(void* userData, ResolveFn resolve) : m_resolve(resolve), m_userData(userData) {}

		void Serialize(const void* pObject, const Reflect::Type& type, IArchiveWriter& writer);
		void Deserialize(void* pObject, const Reflect::Type& type, IArchiveReader& reader);

	private:
		void WriteObject(const void* obj, const Reflect::Type& type, IArchiveWriter& writer);
		void WriteField(const void* valuePtr, const Reflect::Field& field, IArchiveWriter& writer);
		void WriteValue(const void* valuePtr, const Reflect::Field& field, IArchiveWriter& writer);

		void ReadObject(void* obj, const Reflect::Type& type, IArchiveReader& reader);
		void ReadField(void* valuePtr, const Reflect::Field& field, IArchiveReader& reader);
		void ReadValue(void* valuePtr, const Reflect::Field& field, IArchiveReader& reader);

		const Reflect::Type* Resolve(Reflect::TypeHandle handle) { return m_resolve(m_userData, handle); }

	private:
		ResolveFn m_resolve = nullptr;
		void* m_userData = nullptr;
	};
}