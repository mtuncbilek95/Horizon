#pragma once

#include <Engine/Core/ModuleDependency.h>
#include <Engine/Core/ModuleReport.h>
#include <typeindex>

namespace Horizon::Engine
{
	class Engine;

	class Module
	{
		friend class Engine;
	public:
		Module() = default;
		virtual ~Module() = default;

		virtual ModuleReport OnInitialize() = 0;
		virtual void OnFinalize() = 0;
		virtual void DeclareDependencies(ModuleDependency& graph) = 0;

		std::type_index GetTypeId() const
		{
			std::type_index index = typeid(*this);
			return index;
		}

	protected:
		Engine* GetEngine() const noexcept { return m_engine; }

	private:
		Engine* m_engine = nullptr;
		ModuleDependency m_dependency;
	};
}