#include "AssetImportContext.h"

namespace Horizon
{
	AssetImportContext::AssetImportContext(const AssetImportContextDesc& desc, const Guid& guid) : m_metaPath(desc.metaPath), 
		m_cookPath(desc.cookedPath), m_guid(guid)
	{
	}
}