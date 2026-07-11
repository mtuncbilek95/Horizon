#include "AssetImportContext.h"

namespace Horizon
{
	AssetImportContext::AssetImportContext(const std::filesystem::path& source, const Guid& guid) : m_source(source), m_guid(guid)
	{
	}
}