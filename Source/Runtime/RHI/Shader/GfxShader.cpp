#include "GfxShader.h"

#include <cstring>

namespace Horizon::RHI
{
	void GfxShader::SetDebugName(const char* pName)
	{
		if (!pName)
		{
			m_debugName[0] = '\0';
			return;
		}

		std::strncpy(m_debugName, pName, sizeof(m_debugName) - 1);
		m_debugName[sizeof(m_debugName) - 1] = '\0';
	}
}