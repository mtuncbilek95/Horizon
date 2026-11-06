#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <string>

namespace Horizon::PAL
{
	using SymbolAddress = void*;
	using LibInstance = void*;

	class SymbolLibrary
	{
	public:
		SymbolLibrary();
		~SymbolLibrary() = default;

		SymbolLibrary(const SymbolLibrary&) = delete;
		SymbolLibrary& operator=(const SymbolLibrary&) = delete;

		const std::string& GetName() const noexcept { return m_name; }

		SymbolAddress GetSymbol(const std::string& name) const;

	private:
		std::string m_name;
		LibInstance m_instance = nullptr;
	};
}