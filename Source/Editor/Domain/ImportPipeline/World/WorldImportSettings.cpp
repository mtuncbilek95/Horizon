#include "WorldImportSettings.h"

namespace Horizon
{
	void WorldImportSettings::OnImportDefault(MetaHeader& outHeaderToSerialize)
	{
		outHeaderToSerialize.id = Guid::Generate();
		outHeaderToSerialize.cookedPath = std::filesystem::path("Cooked") / (outHeaderToSerialize.id.ToString() + ".hcooked");
		outHeaderToSerialize.assetTypeName = "World";
	}
}