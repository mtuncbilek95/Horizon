#include <Runtime/PAL/Module/SymbolLibrary.h>

#include <Runtime/Log/Terminal.h>

#include <Windows.h>

#include <filesystem>
#include <cassert>

namespace Horizon::PAL
{
	SymbolLibrary::SymbolLibrary(const SymbolLibraryDesc& desc) : m_desc(desc)
	{
		HINSTANCE instance = NULL;
		c8 tempBuffer[MaxNameBufferLength];

		// If its main, GetModuleHandle(), if its not, load the library.
		if (desc.isMain)
		{
			instance = GetModuleHandle(NULL);

			if (!instance)
				return;

			DWORD lengthCopied = GetModuleFileName(instance, tempBuffer, MaxNameBufferLength);

			if (lengthCopied == ERROR_INSUFFICIENT_BUFFER)
				return;
		}
		else
		{
			assert(!desc.path.empty() && "This should be not empty");

			instance = LoadLibrary(desc.path.string().c_str());

			if (!instance)
				return;
		}

		std::filesystem::path newPath = desc.isMain ? std::filesystem::path(tempBuffer) : desc.path;
		std::string name = newPath.filename().string();

		if (name.empty())
		{
			if (desc.isMain)
				return;
			else
			{
				FreeLibrary(instance);
				return;
			}
		}

		m_name = name;
		m_instance = instance;
	}

	SymbolAddress SymbolLibrary::GetSymbol(const std::string& name) const
	{
		if (!m_instance)
		{
			Terminal::Fatal("SymbolLibrary", "{} cant do because instance is empty", name);
			return nullptr;
		}

		return GetProcAddress(HINSTANCE(m_instance), name.c_str());
	}
}