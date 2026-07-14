#include "DomainFile.h"

namespace Horizon
{
	DomainFile::DomainFile(const DomainFileDesc& desc, Engine* pEngine) : m_id(desc.fileId),
		m_parent(desc.pParent), m_engine(pEngine), m_name(desc.fileName)
	{
	}

	DomainFile::~DomainFile()
	{}
}