#pragma once

#include <Engine/Engine/Engine.h>
#include <Engine/System/ISystem.h>
#include <Engine/System/SystemReport.h>

namespace Horizon
{
	template<typename T>
	class System : public ISystem
	{
		friend class Engine;
	public:
		System() = default;
		virtual ~System() = default;

		Engine* GetEngine() const { return m_mainEngine; }
		void SetEngine(Engine* pEngine) { m_mainEngine = pEngine; }

		std::string_view GetObjectType() const final { return TypeName<T>(); }
		std::type_index GetObjectIndex() const final { return std::type_index(typeid(T)); }

		template<typename U, typename = std::enable_if_t<std::derived_from<U, ISystem>>>
		U& RequestSystem()
		{
			if (!m_mainEngine)
				throw std::runtime_error("System is not attached to an engine");

			return m_mainEngine->GetSystem<U>();
		}

	private:
		Engine* m_mainEngine = nullptr;
	};
}