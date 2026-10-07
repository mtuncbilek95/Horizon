#pragma once

#include <Engine/Core/ModuleReport.h>
#include <typeindex>

namespace Horizon::Engine
{
	class Engine;
	class ModuleGraph;
	struct ReflectionLibrary;

	class ENGINE_API Module
	{
		friend class Engine;
	public:
		Module() = default;
		virtual ~Module() = default;

		virtual ModuleReport OnInitialize() = 0;
		virtual void OnFinalize() = 0;
		virtual void DeclareDependencies(ModuleGraph& graph) = 0;

		virtual void OnLibraryRegistered(const ReflectionLibrary& library) = 0;
		virtual void OnLibraryUnregistered(const ReflectionLibrary& library) = 0;

		std::type_index GetTypeId() const
		{
			std::type_index index = typeid(*this);
			return index;
		}

	protected:
		Engine* GetEngine() const noexcept { return m_engine; }

	private:
		Engine* m_engine = nullptr;
	};
}