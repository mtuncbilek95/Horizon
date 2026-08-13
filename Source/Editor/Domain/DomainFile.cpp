#include "DomainFile.h"

#include <Editor/Domain/DomainFolder.h>
#include <Runtime/PAL/File/File.h>

namespace Horizon::Editor
{
	void DomainFile::Rename(const std::string& newName)
	{
		std::string newPath = m_parent->GetAbsolutePath() + "/" + newName;
		PAL::File::Rename(m_sourcePath, newPath);
		// After this, everything should work fine?
	}
}