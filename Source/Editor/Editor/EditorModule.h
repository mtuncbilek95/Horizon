#pragma once

#include <Engine/Engine/IModule.h>

#include <memory>

namespace Horizon
{
	class GfxDevice;
	class MainScreen;

	class EditorModule : public IModule
	{
	public:
		EditorModule();
		~EditorModule();

		void OnAttach(Engine& engine) final;
		void OnSync() final;
		void OnDetach() final;

	private:
		GfxDevice* m_device = nullptr;
		std::unique_ptr<MainScreen> m_mainScreen;
	};
}
