#pragma once

#include <Engine/Asset/AssetKind.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	class ENGINE_API AssetTypeAttribute final : public Reflect::Attribute
	{
		HORIZON_ATTRIBUTE_REFLECT(AssetTypeAttribute);
	public:
		AssetTypeAttribute(AssetKind kind) : m_kind(kind)
		{
		}
		~AssetTypeAttribute() = default;

		AssetKind GetKind() const { return m_kind; }

	private:
		AssetKind m_kind;
	};
}