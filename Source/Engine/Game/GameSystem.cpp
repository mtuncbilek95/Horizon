#include "GameSystem.h"

#include "GameSystem.h"

#include <Engine/Core/Engine.h>
#include <Runtime/Containers/StringOps.h>
#include <Runtime/Log/Terminal.h>
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

		m_moduleName = moduleName;
		m_libraryPath = std::format("{}/Binaries/{}/{}.dll", StringOps::NormalizePath(projectRoot), BuildConfigName(), moduleName);

		if (!PAL::File::Exists(m_libraryPath))
			return Fail("Library is not built");

		PAL::SymbolLibraryDesc desc;
		desc.path = m_libraryPath;
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

	void GameSystem::Unload()
	{
		if (!m_gameLibrary)
			return;

		m_engine->UnregisterLibrary(m_gameLibrary);
		Memory::Allocator::Delete(m_gameLibrary);
		m_gameLibrary = nullptr;
		m_state = GameModuleState::None;

		Terminal::Info(StringOps::GetName(this), "{} unloaded", m_moduleName);
	}

	b8 GameSystem::Fail(const std::string& reason)
	{
		m_state = GameModuleState::Failed;
		m_failReason = reason;

		Terminal::Error(StringOps::GetName(this), "{} could not be loaded from {}, {}", m_moduleName, m_libraryPath, reason);
		return false;
	}
}