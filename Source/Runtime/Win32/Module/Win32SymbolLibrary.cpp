#include <Runtime/PAL/Module/SymbolLibrary.h>

#include <Runtime/Log/Terminal.h>

#include <Windows.h>

namespace Horizon::PAL
{
	SymbolLibrary::SymbolLibrary()
	{
		HINSTANCE instance = GetModuleHandle(NULL);

		if (!instance)
			return;

		c8 tempBuffer[MaxNameBufferLength];
		DWORD lengthCopied = GetModuleFileName(instance, tempBuffer, MaxNameBufferLength);

		if (lengthCopied == 0 || lengthCopied == MaxNameBufferLength)
			return;

		m_name = std::string(tempBuffer, lengthCopied);
		m_instance = instance;
	}

	SymbolAddress SymbolLibrary::GetSymbol(const std::string& name) const
	{
		if (!m_instance)
		{
			Terminal::Fatal("SymbolLibrary", "{} cant do because instance is empty", name);
			return nullptr;
		}

		return GetProcAddress(HINSTANCE(m_instance), name.data());
	}
}