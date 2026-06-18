#pragma once

#include <Runtime/Containers/Guid.h>

#include <string>
#include <vector>

namespace Horizon
{
	class AssetModule;

	struct AssetLoadContext
	{
		AssetModule* pModule = nullptr;
		Guid guid;
		std::string cookedPath;
		const std::vector<Guid>* dependencies = nullptr;
	};

	class IAssetLoader
	{
	public:
		virtual ~IAssetLoader() = default;

		virtual void* Load(const AssetLoadContext& ctx) = 0;
		virtual void Unload(void* asset) = 0;
	};

	template<typename T>
	class TAssetLoader : public IAssetLoader
	{
	public:
		void* Load(const AssetLoadContext& ctx) final { return LoadTyped(ctx); }
		void  Unload(void* asset) final { DestroyTyped(static_cast<T*>(asset)); }

	protected:
		virtual T* LoadTyped(const AssetLoadContext& ctx) = 0;
		virtual void DestroyTyped(T* asset) { delete asset; }
	};
}