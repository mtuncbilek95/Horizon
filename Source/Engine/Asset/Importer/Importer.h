#pragma once

#include <filesystem>

namespace Horizon
{
	template<typename TAsset>
	class Importer
	{
	public:
		virtual ~Importer() = default;

		virtual bool Import(TAsset& asset, const std::filesystem::path& path) = 0;
	};
}
