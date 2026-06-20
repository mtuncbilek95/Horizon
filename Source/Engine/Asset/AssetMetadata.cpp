#include "AssetMetadata.h"

#include <fstream>

namespace Horizon
{
	void AssetMetadata::Serialize(const std::filesystem::path& toWhere)
	{
		json generalInfo = {};
		generalInfo["assetId"] = assetId.ToString();
		generalInfo["assetName"] = assetName;
		generalInfo["assetType"] = assetType;
		generalInfo["binPath"] = binPath;

		json typeInformation;
		if (assetInfo)
			assetInfo->Serialize(typeInformation);

		json jsonFile;
		jsonFile["generalInformation"] = generalInfo;
		jsonFile["typeInformation"] = typeInformation;

		std::filesystem::path metaPath = toWhere / (assetName + ".hmeta");

		std::ofstream file(metaPath.string());
		file << jsonFile.dump(2);
		file.close();
	}

	void AssetMetadata::Deserialize(const std::filesystem::path& fromWhere)
	{
	}
}