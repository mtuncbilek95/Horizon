#pragma once

#include <Engine/Object/Object.h>

namespace Horizon
{
	template<typename T>
	class Asset : public Object<T>
	{
	public:
		Asset() = default;
		virtual ~Asset() = default;

		virtual bool Load() = 0;
		virtual void Unload() = 0;
		virtual bool IsLoaded() const = 0;

		const std::filesystem::path& AssetPath() const { return m_assetPath; }

	protected:
		std::filesystem::path m_assetPath;
	};
}