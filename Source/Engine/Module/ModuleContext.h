#pragma once

#include <Engine/Core/Context.h>

#include <Runtime/Containers/List.h>
#include <Runtime/PAL/Module/SymbolLibrary.h>
#include <Runtime/RTTR/Reflection.h>

#include <unordered_map>

namespace Horizon
{
	class ModuleContext : public Context 
	{
	public:
		ModuleContext(PAL::SymbolLibrary* pLibrary);
		~ModuleContext() = default;

		EngineReport OnAttach(Engine* pEngine) final;
		void OnDetach() final;

		Reflect::Type* GetType(Reflect::TypeHandle handl);
		List<Reflect::Type*> GetTypeByBase(Reflect::TypeHandle handl);
		List<Reflect::Type*> GetTypeByAttribute(Reflect::TypeHandle attrHandle);

	private:
		List<Reflect::Type> m_registeredTypes;
		std::unordered_map<Reflect::TypeHandle, u32> m_lookup;

		PAL::SymbolLibrary* m_module = nullptr;
	};
}