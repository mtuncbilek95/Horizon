#pragma once

#include <Engine/Asset/AssetObject.h>

namespace Horizon::Engine
{
	HCLASS();
	class H_EXPORT MeshAsset : public AssetObject
	{
		HORIZON_TYPE_REFLECT(MeshAsset);
	public:
		MeshAsset() = default;
		~MeshAsset() = default;

		void BeginUse();
		void EndUse();

	private:
	};
}