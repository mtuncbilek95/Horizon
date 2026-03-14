#pragma once

#include <Engine/Asset/Asset.h>

namespace Horizon
{
	class MaterialImporter;

	class MaterialAsset : public Asset<MaterialAsset>
	{
	public:
		MaterialAsset() = default;
		virtual ~MaterialAsset() = default;

		bool Load() final;
		void Unload() final;
		bool IsLoaded() const final { return m_loaded; }

	private:
		bool m_loaded = false;

		friend class MaterialImporter;
	};
}
