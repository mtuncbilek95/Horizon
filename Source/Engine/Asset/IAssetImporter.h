#pragma once

namespace Horizon
{
	class IAssetLoader
	{
	public:
		virtual ~IAssetLoader() = default;

		virtual void* Load(const AssetImporterSettings& settings, const u8* payload, usize size) = 0;
		virtual void Unload(void* asset) = 0;
	};
}