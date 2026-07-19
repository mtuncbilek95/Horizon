#pragma once

namespace Horizon
{
	class AssetLifetimeStrategy
	{
	public:
		virtual ~AssetLifetimeStrategy() = default;

		virtual void Load() = 0; // TODO: Add something here later
		virtual void Unload() = 0; // TODO: Add something here later
	};
}