#pragma once

#include <Engine/Engine/IModule.h>

namespace Horizon
{
	class AssetModule final : public IModule
	{
	public:
		void OnAttach(Engine& engine);
		void OnSync();
		void OnDetach();

	private:
	};
}