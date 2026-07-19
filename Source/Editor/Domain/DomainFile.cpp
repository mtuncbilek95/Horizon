#include "DomainFile.h"

#include <Engine/Core/Engine.h>
#include <Engine/Asset/AssetSystem.h>
#include <Runtime/PAL/File/File.h>

#include <nlohmann/json.hpp>

namespace Horizon
{
	DomainFile::DomainFile(const DomainFileDesc& desc, Engine* pEngine)
		: m_engine(pEngine), m_parentFolder(desc.parentFolder), m_metaPath(desc.metaPath)
	{
	}

	DomainFile::~DomainFile()
	{
	}
}