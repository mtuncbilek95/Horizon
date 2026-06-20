#pragma once

#include <nlohmann/json.hpp>

#include <string>
#include <filesystem>
#include <iosfwd>

using json = nlohmann::json;

namespace Horizon
{
	struct AssetProperties
	{
		virtual ~AssetProperties() = default;

		virtual void Serialize(json& outFile) const = 0;
		virtual void Deserialize(const json& inFile) = 0;
		virtual void Import(const std::filesystem::path& path) = 0;
		virtual void Cook(std::ostream& out) const = 0;
	};

	struct AssetMetadata
	{
		Guid assetId;
		std::string assetName;
		std::string assetType;

		usize binarySize;
		std::filesystem::path binPath;

		AssetProperties* assetInfo = nullptr;

		void Serialize(const std::filesystem::path& toWhere);
		void Deserialize(const std::filesystem::path& fromWhere);
	};

	using PropertyFactory = AssetProperties* (*)();
}