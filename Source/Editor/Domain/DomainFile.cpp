#include "DomainFile.h"

namespace Horizon
{
	DomainFile::DomainFile(const DomainFileDesc& desc, Engine* pEngine) : m_id(desc.fileId),
		m_parent(desc.pParent), m_metaPath(desc.metaPath), m_binaryPath(desc.binaryPath),
		m_engine(pEngine)
	{
		m_name = m_metaPath.filename().string();
		if(std::filesystem::exists(m_binaryPath))
			m_binarySize = std::filesystem::file_size(m_binaryPath);
	}

	DomainFile::~DomainFile()
	{}
}