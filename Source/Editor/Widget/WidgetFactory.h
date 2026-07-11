#pragma once

#include <Runtime/Containers/Singleton.h>

#include <vector>
#include <unordered_map>
#include <typeindex>

namespace Horizon
{
	class Engine;
	class IWidget;

	enum class WidgetDock : u8
	{
		Center,
		Left,
		Right,
		Bottom
	};

	struct WidgetTypeInfo
	{
		std::string name;
		std::string_view icon;
		WidgetDock dock;
		IWidget* (*CreateWidget)(Engine* engine);
		b8 alwaysOpenFirst;
		std::type_index type;
	};

	class WidgetFactory : public Singleton<WidgetFactory>
	{
	public:
		void Register(const WidgetTypeInfo& info) 
		{ 
			m_types.push_back(info);
			m_lookup[info.type] = m_types.size() - 1;
			
			Terminal::Log("WidgetFactory", "{} has been registered", info.name);
		}

		template<typename Fn>
		void ForEach(Fn&& func) const 
		{
			for (const auto& info : m_types)
				func(info);
		}

		WidgetTypeInfo& GetByInfo(const std::type_index& info)
		{
			auto it = m_lookup.find(info);
			Terminal::Assert(it != m_lookup.end(), "WidgetFactory", "Cant find the correct type");

			return m_types[it->second];
		}

	private:
		std::vector<WidgetTypeInfo> m_types;
		std::unordered_map<std::type_index, usize> m_lookup;
	};
}