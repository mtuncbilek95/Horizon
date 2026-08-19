#pragma once

#include <Runtime/Containers/List.h>
#include <Runtime/PAL/Module/SymbolLibrary.h>
#include <Runtime/RTTR/Reflection.h>

#include <unordered_map>

namespace Horizon::Engine
{
	class Engine;

	class ReflectionSystem final
	{
	public:
		ReflectionSystem(Engine* pEngine);
		~ReflectionSystem();

		Reflect::Type* GetType(Reflect::TypeHandle handl);
		List<Reflect::Type*> GetTypeByBase(Reflect::TypeHandle handl);
		List<Reflect::Type*> GetTypeByAttribute(Reflect::TypeHandle attrHandle);

	private:
		List<Reflect::Type> m_registeredTypes;
		std::unordered_map<Reflect::TypeHandle, u32> m_lookup;

		PAL::SymbolLibrary* m_hostLibrary = nullptr;
	};
}