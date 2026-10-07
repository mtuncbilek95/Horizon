#pragma once

#include <Engine/Reflection/ReflectionLibrary.h>
#include <Runtime/Containers/List.h>
#include <Runtime/PAL/Module/SymbolLibrary.h>
#include <Runtime/RTTR/Reflection.h>

#include <string>
#include <unordered_map>

namespace Horizon::Engine
{
	class Engine;

	class ENGINE_API ReflectionSystem final
	{
	public:
		ReflectionSystem(Engine* pEngine);
		~ReflectionSystem();

		ReflectionSystem(const ReflectionSystem&) = delete;
		ReflectionSystem& operator=(const ReflectionSystem&) = delete;

		b8 RegisterLibrary(const PAL::SymbolLibrary* pLibrary);
		void UnregisterLibrary(const PAL::SymbolLibrary* pLibrary);

		Reflect::Type* GetType(Reflect::TypeHandle handl);
		Reflect::Type* GetTypeByName(const std::string& name);
		List<Reflect::Type*> GetTypeByBase(Reflect::TypeHandle handl);
		List<Reflect::Type*> GetTypeByAttribute(Reflect::TypeHandle attrHandle);

		ReflectionLibrary* FindLibrary(const PAL::SymbolLibrary* pLibrary);

	private:
		void IndexType(Reflect::Type& type);
		void UnindexType(Reflect::Type& type);

	private:
		List<ReflectionLibrary> m_libraries;
		std::unordered_map<Reflect::TypeHandle, Reflect::Type*> m_typeLookup;
		std::unordered_map<std::string, Reflect::Type*> m_nameLookup;
		std::unordered_map<Reflect::TypeHandle, List<Reflect::Type*>> m_byBase;
		std::unordered_map<Reflect::TypeHandle, List<Reflect::Type*>> m_byAttribute;

		PAL::SymbolLibrary* m_hostLibrary = nullptr;
	};
}