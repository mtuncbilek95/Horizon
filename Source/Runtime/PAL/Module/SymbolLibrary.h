#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

#include <filesystem>
#include <string>

namespace Horizon::PAL
{
	using SymbolAddress = void*;
	using LibInstance = void*;

	struct SymbolLibraryDesc
	{
		std::filesystem::path path;
		b8 isMain = true;
	};

	class H_EXPORT SymbolLibrary
	{
	public:
		SymbolLibrary(const SymbolLibraryDesc& desc);
		~SymbolLibrary() = default;

		const std::filesystem::path& GetPath() const noexcept { return m_desc.path; }
		b8 GetIsMain() const noexcept { return m_desc.isMain; }
		const std::string& GetName() const noexcept { return m_name; }

		SymbolAddress GetSymbol(const std::string& name) const;

	private:
		SymbolLibraryDesc m_desc;
		std::string m_name;

		LibInstance m_instance;
	};
}