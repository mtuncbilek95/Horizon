#pragma once

#include <Runtime/Serialization/Archive.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon
{
	class H_EXPORT Serializer
	{
	public:
		using ResolveFn = const Reflect::Type* (*)(void* pUserData, Reflect::TypeHandle handle);
		using ResolveNameFn = const Reflect::Type* (*)(void* pUserData, const std::string& name);

		Serializer(void* pUserData, ResolveFn resolve, ResolveNameFn resolveName = nullptr) : m_resolve(resolve),
			m_resolveName(resolveName), m_userData(pUserData)
		{
		}

		void Serialize(const void* pObject, const Reflect::Type& type, IArchiveWriter& writer);
		void Deserialize(void* pObject, const Reflect::Type& type, IArchiveReader& reader);

	private:
		void WriteObject(const void* pObject, const Reflect::Type& type, IArchiveWriter& writer);
		void WriteField(const void* pValue, const Reflect::Field& field, IArchiveWriter& writer);
		void WriteValue(const void* pValue, const Reflect::Field& field, IArchiveWriter& writer);
		void WritePointer(const void* pPointerSlot, const Reflect::Field& field, IArchiveWriter& writer);

		void ReadObject(void* pObject, const Reflect::Type& type, IArchiveReader& reader);
		void ReadField(void* pValue, const Reflect::Field& field, IArchiveReader& reader);
		void ReadValue(void* pValue, const Reflect::Field& field, IArchiveReader& reader);
		void ReadPointer(void* pPointerSlot, const Reflect::Field& field, IArchiveReader& reader);

		b8 SeekField(const Reflect::Field& field, IArchiveReader& reader);

		const Reflect::Type* Resolve(Reflect::TypeHandle handle) { return m_resolve(m_userData, handle); }
		const Reflect::Type* ResolveByName(const std::string& name) { return m_resolveName ? m_resolveName(m_userData, name) : nullptr; }

	private:
		ResolveFn m_resolve = nullptr;
		ResolveNameFn m_resolveName = nullptr;
		void* m_userData = nullptr;
	};
}