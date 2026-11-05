#pragma once

#include <Runtime/Containers/Vector/List.h>

#include <typeindex>

namespace Horizon::Engine
{
	class ModuleDependency
	{
		friend class Engine;
	public:
		template<typename... Ts>
		void InitBefore()
		{
			(m_initBeforeList.PushBack(typeid(Ts)), ...);
		}

		template<typename... Ts>
		void InitAfter()
		{
			(m_initAfterList.PushBack(typeid(Ts)), ...);
		}

		template<typename... Ts>
		void RunBefore()
		{
			(m_runBeforeList.PushBack(typeid(Ts)), ...);
		}

		template<typename... Ts>
		void RunAfter()
		{
			(m_runAfterList.PushBack(typeid(Ts)), ...);
		}

	private:
		List<std::type_index> m_initBeforeList;
		List<std::type_index> m_initAfterList;
		List<std::type_index> m_runBeforeList;
		List<std::type_index> m_runAfterList;
	};
}