#pragma once

#include <Engine/Engine/IModule.h>

namespace Horizon
{
	class EditorModule : public IModule
	{
	public:
		void OnAttach(Engine& engine) final;
		void OnSync() final;
		void OnDetach() final;

	private:
	};
}