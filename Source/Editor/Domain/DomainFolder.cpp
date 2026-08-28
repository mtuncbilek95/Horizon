#include "DomainFolder.h"

#include <Runtime/PAL/File/Directory.h>

namespace Horizon::Editor
{
	void DomainFolder::Rename(const std::string& newName)
	{
		std::string newPath = m_parent->GetAbsolutePath() + "/" + newName;
		PAL::Directory::Rename(m_absolutePath, newPath);
		// TODO: After this, everything should work fine?
	}
}