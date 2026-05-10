#pragma once

#include <Engine/Core/Engine.h>
#include <Engine/Core/IContext.h>
#include <Engine/Core/ISystem.h>

namespace Horizon
{
	template<typename T>
	class Context : public IContext
	{
		friend class Engine;
	public:
		Context() = default;
		virtual ~Context() = default;

		Engine* GetEngine() const { return m_mainEngine; }
		void SetEngine(Engine* pEngine) { m_mainEngine = pEngine; }

		std::string GetObjectType() const final { return TypeName<T>(); }
		std::type_index GetObjectIndex() const final { return std::type_index(typeid(T)); }

		template<typename U, typename = std::enable_if_t<std::derived_from<U, ISystem>>>
		U& RequestSystem()
		{
			if (!m_mainEngine)
				throw std::runtime_error("System is not attached to an engine");

			return m_mainEngine->GetSystem<U>();
		}

		template<typename U, typename = std::enable_if_t<std::derived_from<U, IContext>>>
		U& RequestContext()
		{
			if (!m_mainEngine)
				throw std::runtime_error("Context is not attached to an engine");

			return m_mainEngine->GetContext<U>();
		}

	private:
		Engine* m_mainEngine = nullptr;
	};
}