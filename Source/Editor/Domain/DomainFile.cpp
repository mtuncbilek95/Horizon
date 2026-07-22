// Editor/Domain/DomainFile.cpp
#include "DomainFile.h"

#include <Runtime/PAL/File/File.h>
#include <Runtime/Log/Terminal.h>

#include <nlohmann/json.hpp>

namespace Horizon
{
	DomainFile::DomainFile(const DomainFileDesc& desc, Engine* pEngine)
		: m_engine(pEngine), m_parentFolder(desc.parentFolder),
		m_metaPath(desc.metaPath), m_name(desc.name)
	{
	}

	DomainFile::~DomainFile()
	{
		// TODO: This should be finished up — AssetSystem::Unregister?
	}
}