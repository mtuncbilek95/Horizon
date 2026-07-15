#pragma once

#include <Engine/Core/Context.h>

#include <Runtime/PAL/Module/SymbolLibrary.h>
#include <Runtime/Reflection/TypeManifest.h>

#include <vector>
#include <unordered_map>

namespace Horizon
{
	class ModuleContext : public Context 
	{
	public:
		EngineReport OnAttach(Engine*) final;
		void OnDetach() final;

		TypeManifest* GetManifest(ReflectionTypeHandle handl);
		std::vector<TypeManifest*> GetManifestsByBase(ReflectionTypeHandle handl);
		
	private:
		std::vector<TypeManifest> m_manifests;
		std::unordered_map<ReflectionTypeHandle, u32> m_lookup;

		PAL::SymbolLibrary* m_module = nullptr;
	};
}