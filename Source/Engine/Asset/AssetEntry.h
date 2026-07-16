#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace Horizon
{
	class IAsset;

	enum class AssetState : u8
	{
		Registered,
		Loading,
		Loaded,
		Failed
	};

	class AssetEntry
	{
		friend class AssetSystem;
	public:
		const Guid& GetGuid() const { return m_guid; }
		std::string_view GetAssetType() const { return m_type; }
		const std::filesystem::path& GetCookedPath() const { return m_cookedPath; }
		const std::vector<Guid>& GetDependencies() const { return m_dependencies; }

		u64 GetSettingsOffset() const { return m_settingsOffset; }
		u64 GetSettingsSize() const { return m_settingsSize; }
		u64 GetPayloadOffset() const { return m_payloadOffset; }
		u64 GetPayloadSize() const { return m_payloadSize; }

		AssetState GetState() const { return m_state; }
		b8 IsLoaded() const { return m_state == AssetState::Loaded; }
		IAsset* GetRuntime() const { return m_runtime; }

	private:
		Guid m_guid;
		std::string m_type;
		std::filesystem::path m_cookedPath;
		std::vector<Guid> m_dependencies;

		u64 m_settingsOffset = 0, m_settingsSize = 0;
		u64 m_payloadOffset = 0, m_payloadSize = 0;

		IAsset* m_runtime = nullptr;
		AssetState m_state = AssetState::Registered;
	};
}