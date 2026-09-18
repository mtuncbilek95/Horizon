#pragma once

#include <Engine/Asset/AssetEntry.h>
#include <Runtime/Containers/Guid.h>

#include <string_view>

namespace Horizon::Engine
{
	class H_EXPORT AssetSourceFile
	{
	public:
		virtual ~AssetSourceFile() = default;

		virtual std::string_view GetName() const = 0;

		virtual const AssetEntry* Find(const Guid& id) const = 0;
	};
}