#pragma once

#include <typeindex>
#include <unordered_map>

namespace Horizon
{
	class ServiceLocator
	{
	public:

	private:
		std::unordered_map<std::type_index, std::any> m_services;
		std::unordered_map<std::type_index, std::function<std::any()>> m_factories;
	};
}