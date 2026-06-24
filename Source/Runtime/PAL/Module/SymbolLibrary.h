#pragma once

#include <Runtime/Containers/Path.h>

namespace Horizon
{
	using SymbolAddress = void*;
	using LibInstance = void*;

	struct SymbolLibraryDesc
	{
		Path path;
		b8 isMain = true;
	};

	class H_EXPORT SymbolLibrary
	{
	public:
		SymbolLibrary(const SymbolLibraryDesc& desc);
		~SymbolLibrary() = default;

		const Path& GetPath() const noexcept { return m_desc.path; }
		b8 GetIsMain() const noexcept { return m_desc.isMain; }
		const String& GetName() const noexcept { return m_name; }

		SymbolAddress GetSymbol(const String& name) const;

	private:
		SymbolLibraryDesc m_desc;
		String m_name;

		LibInstance m_instance;
	};
}