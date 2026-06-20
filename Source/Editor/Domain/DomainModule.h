#pragma once

#include <Engine/Core/Submodule.h>

namespace Horizon
{
	class Window;

	class DomainModule final : public Submodule
	{
	public:
		DomainModule() = default;
		~DomainModule() = default;

		void OnAttach(Engine* engine) final;
		void OnSync() final;
		void OnDetach() final;

	private:

	};
}