#pragma once

#include <string>

namespace Horizon
{
	using SymbolAddress = void*;
	using LibInstance = void*;

	struct SymbolLibraryDesc
	{
		std::string path;
		b8 isMain = true;
	};

	class H_EXPORT SymbolLibrary
	{
	public:
		SymbolLibrary(const SymbolLibraryDesc& desc);
		~SymbolLibrary() = default;

		const std::string& GetPath() const noexcept { return m_desc.path; }
		b8 GetIsMain() const noexcept { return m_desc.isMain; }
		const std::string& GetName() const noexcept { return m_name; }

		SymbolAddress GetSymbol(const std::string& name) const;

	private:
		SymbolLibraryDesc m_desc;
		std::string m_name;

		LibInstance m_instance;
	};
}