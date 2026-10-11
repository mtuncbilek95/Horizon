#pragma once

#include <Engine/Core/Context.h>

struct lua_State;

namespace Horizon::Engine
{
	class ENGINE_API ScriptContext final : public Context
	{
	public:
		ModuleReport OnInitialize() final;
		void OnFinalize() final;
		void DeclareDependencies(ModuleGraph& graph) final;

		void OnLibraryRegistered(const ReflectionLibrary& library) final;
		void OnLibraryUnregistered(const ReflectionLibrary& library) final;

		void Reset();
		lua_State* GetState() const;

	private:
		void CreateState();
		void DestroyState();

	private:
		struct Impl;
		Impl* m_impl = nullptr;
	};
}