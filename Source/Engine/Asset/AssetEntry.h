#pragma once

#include <filesystem>
#include <string>

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

		u64 GetDepsOffset() const { return m_depsOffset; }
		u32 GetDepsCount() const { return m_depsCount; }

		u64 GetDescriptorOffset() const { return m_descriptorOffset; }
		u64 GetDescriptorSize() const { return m_descriptorSize; }
		u64 GetPayloadOffset() const { return m_payloadOffset; }
		u64 GetPayloadSize() const { return m_payloadSize; }

		AssetState GetState() const { return m_state; }
		b8 IsLoaded() const { return m_state == AssetState::Loaded; }
		IAsset* GetRuntime() const { return m_runtime; }

	private:
		Guid m_guid;
		std::string m_type;
		std::filesystem::path m_cookedPath;

		u64 m_depsOffset = 0;
		u32 m_depsCount = 0;

		u64 m_descriptorOffset = 0, m_descriptorSize = 0;
		u64 m_payloadOffset = 0, m_payloadSize = 0;

		IAsset* m_runtime = nullptr;
		AssetState m_state = AssetState::Registered;
	};
}