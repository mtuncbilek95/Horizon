#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/PAL/Module/SymbolLibrary.h>

#include <string>

namespace Horizon::Engine
{
	class Engine;

	enum class GameModuleState : u8
	{
		None,
		Loaded,
		Failed
	};

	class ENGINE_API GameSystem
	{
	public:
		GameSystem(Engine* pEngine);
		~GameSystem();

		GameSystem(const GameSystem&) = delete;
		GameSystem& operator=(const GameSystem&) = delete;

		b8 Load(const std::string& projectRoot, const std::string& moduleName);
		void Unload();
		b8 Reload();
		void PollReload();

		GameModuleState GetState() const { return m_state; }
		b8 IsLoaded() const { return m_state == GameModuleState::Loaded; }

		const std::string& GetModuleName() const { return m_moduleName; }
		const std::string& GetLibraryPath() const { return m_libraryPath; }
		const std::string& GetFailReason() const { return m_failReason; }

	private:
		b8 LoadShadowCopy();
		b8 Fail(const std::string& reason);

	private:
		Engine* m_engine = nullptr;
		PAL::SymbolLibrary* m_gameLibrary = nullptr;

		std::string m_moduleName;
		std::string m_libraryPath;
		std::string m_markerPath;
		std::string m_shadowRoot;
		std::string m_shadowPath;
		std::string m_failReason;

		u64 m_loadedStamp = 0;
		b8 m_reloadDeferred = false;

		GameModuleState m_state = GameModuleState::None;
	};
}