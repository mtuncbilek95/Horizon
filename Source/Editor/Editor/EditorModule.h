#pragma once

#include <Engine/Core/Submodule.h>

namespace Horizon
{
	class Window;

	class EditorModule final : public Submodule
	{
	public:
		EditorModule() = default;
		~EditorModule() = default;

		void OnAttach(Engine* engine) final;
		void OnSync() final;
		void OnDetach() final;

	private:
	};
}