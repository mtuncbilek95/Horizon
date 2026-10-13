#include "GameSystem.h"

#include <Engine/Core/Engine.h>
#include <Engine/World/SystemGroup.h>
#include <Engine/World/WorldService.h>
#include <Runtime/Containers/StringOps.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/PAL/File/Directory.h>
#include <Runtime/PAL/File/File.h>

#include <format>

namespace Horizon::Engine
{
	namespace
	{
		constexpr const c8* BuildConfigName()
		{
#if defined(HORIZON_DEBUG)
			return "Debug";
#else
			return "Release";
#endif
		}
	}

	GameSystem::GameSystem(Engine* pEngine) : m_engine(pEngine)
	{
	}

	GameSystem::~GameSystem()
	{
		Unload();
	}

	b8 GameSystem::Load(const std::string& projectRoot, const std::string& moduleName)
	{
		if (m_state == GameModuleState::Loaded)
		{
			Terminal::Warn(StringOps::GetName(this), "{} is already loaded", m_moduleName);
			return true;
		}

		const std::string root = StringOps::NormalizePath(projectRoot);
		const std::string binaries = std::format("{}/Binaries/{}", root, BuildConfigName());

		m_moduleName = moduleName;
		m_libraryPath = std::format("{}/{}.dll", binaries, moduleName);
		m_markerPath = std::format("{}/{}.reload", binaries, moduleName);
		m_shadowRoot = std::format("{}/Intermediate/HotReload", root);
		m_shadowPath = std::format("{}/{}.dll", m_shadowRoot, moduleName);

		if (!PAL::File::Exists(m_libraryPath))
			return Fail("Library is not built");

		if (!PAL::Directory::Exists(m_shadowRoot) && !PAL::Directory::Create(m_shadowRoot))
			return Fail("Shadow root could not be created");

		return LoadShadowCopy();
	}

	void GameSystem::Unload()
	{
		if (!m_gameLibrary)
			return;

		m_engine->UnregisterLibrary(m_gameLibrary);
		Memory::Allocator::Delete(m_gameLibrary);
		m_gameLibrary = nullptr;
		m_state = GameModuleState::None;

		if (!PAL::File::Delete(m_shadowPath))
			Terminal::Warn(StringOps::GetName(this), "{} could not be deleted, library may still be alive", m_shadowPath);

		Terminal::Info(StringOps::GetName(this), "{} unloaded", m_moduleName);
	}

	b8 GameSystem::Reload()
	{
		if (m_moduleName.empty())
		{
			Terminal::Warn(StringOps::GetName(this), "Reload requested before any load");
			return false;
		}

		Unload();
		return LoadShadowCopy();
	}

	void GameSystem::PollReload()
	{
		if (m_moduleName.empty())
			return;

		const u64 stamp = PAL::File::GetLastWriteTime(m_markerPath);

		if (stamp == 0 || stamp == m_loadedStamp)
			return;

		auto* pWorld = m_engine->RequestService<WorldService>();
		const SystemGroup running = pWorld ? pWorld->GetRunningSystems() : SystemGroup::None;

		if (running == SceneGroups::Play || running == SceneGroups::Pause)
		{
			if (!m_reloadDeferred)
			{
				Terminal::Info(StringOps::GetName(this), "New {} build is waiting for edit mode", m_moduleName);
				m_reloadDeferred = true;
			}
			return;
		}

		m_reloadDeferred = false;
		Reload();
	}

	b8 GameSystem::LoadShadowCopy()
	{
		// This will help to check if the game dll has been renewed
		m_loadedStamp = PAL::File::GetLastWriteTime(m_markerPath);

		if (!PAL::File::Copy(m_libraryPath, m_shadowRoot))
			return Fail("Library could not be shadow copied, previous copy may still be loaded");

		PAL::SymbolLibraryDesc desc;
		desc.path = m_shadowPath;
		desc.isMain = false;

		m_gameLibrary = Memory::Allocator::Create<PAL::SymbolLibrary>(Memory::CurrLoc(), desc);

		if (m_gameLibrary->GetName().empty())
		{
			Memory::Allocator::Delete(m_gameLibrary);
			m_gameLibrary = nullptr;
			return Fail("Library could not be loaded");
		}

		if (!m_engine->RegisterLibrary(m_gameLibrary))
		{
			Memory::Allocator::Delete(m_gameLibrary);
			m_gameLibrary = nullptr;
			return Fail("Manifestation could not be registered");
		}

		m_state = GameModuleState::Loaded;
		m_failReason.clear();

		Terminal::Info(StringOps::GetName(this), "{} loaded from {}", m_moduleName, m_libraryPath);
		return true;
	}

	b8 GameSystem::Fail(const std::string& reason)
	{
		m_state = GameModuleState::Failed;
		m_failReason = reason;

		Terminal::Error(StringOps::GetName(this), "{} could not be loaded from {}, {}", m_moduleName, m_libraryPath, reason);
		return false;
	}
}