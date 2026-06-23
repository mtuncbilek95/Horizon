#pragma once

#include <Engine/Core/Submodule.h>

namespace Horizon
{
	class ReflectionModule final : public Submodule
	{
	public:
		void OnAttach(Engine* pEngine) final;
		void OnSync() final;
		void OnDetach() final;

	private:
	};
}