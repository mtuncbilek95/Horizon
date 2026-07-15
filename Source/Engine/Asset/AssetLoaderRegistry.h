#pragma once

namespace Horizon
{
	struct AssetLoaderTypeInfo
	{
		std::string_view type;

		IAssetLoader* (*CreateLoader)();
	};

	class AssetLoaderRegistry : public Singleton<AssetLoaderRegistry>
	{
	public:
		void Register(const AssetLoaderTypeInfo& info)
		{
			m_byType[std::string(info.type)] = info;
			Terminal::Log("AssetLoaderRegistry", "Loader registered for type {}", info.type);
		}

		const AssetLoaderTypeInfo* Find(std::string_view type) const 
		{
			auto it = m_byType.find(std::string(type));
			return it == m_byType.end() ? nullptr : &it->second;
		}

	private:
		std::unordered_map<std::string, AssetLoaderTypeInfo> m_byType;
	};
}