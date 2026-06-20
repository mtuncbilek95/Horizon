#pragma once

#include <Engine/Core/Submodule.h>

namespace Horizon
{
	class Window;

	class ProjectModule final : public Submodule
	{
	public:
		ProjectModule() = default;
		~ProjectModule() = default;

		void OnAttach(Engine* engine) final;
		void OnSync() final;
		void OnDetach() final;

	private:
	};
}