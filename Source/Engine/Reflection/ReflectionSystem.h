#pragma once

#include <Runtime/Containers/Vector/List.h>
#include <Runtime/PAL/Module/SymbolLibrary.h>
#include <Runtime/RTTR/Reflection.h>

#include <string>
#include <unordered_map>

namespace Horizon::Engine
{
	class Engine;

	class ReflectionSystem final
	{
	public:
		ReflectionSystem(Engine* pEngine);
		~ReflectionSystem() = default;

		ReflectionSystem(const ReflectionSystem&) = delete;
		ReflectionSystem& operator=(const ReflectionSystem&) = delete;

		Reflect::Type* GetType(Reflect::TypeHandle handl);
		Reflect::Type* GetTypeByName(const std::string& name);
		List<Reflect::Type*> GetTypeByBase(Reflect::TypeHandle handl);
		List<Reflect::Type*> GetTypeByAttribute(Reflect::TypeHandle attrHandle);

	private:
		void IndexType(Reflect::Type& type);

	private:
		List<Reflect::Type> m_types;
		std::unordered_map<Reflect::TypeHandle, Reflect::Type*> m_typeLookup;
		std::unordered_map<std::string, Reflect::Type*> m_nameLookup;
		std::unordered_map<Reflect::TypeHandle, List<Reflect::Type*>> m_byBase;
		std::unordered_map<Reflect::TypeHandle, List<Reflect::Type*>> m_byAttribute;
	};
}