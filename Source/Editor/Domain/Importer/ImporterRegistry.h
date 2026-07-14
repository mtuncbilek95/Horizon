#pragma once

#include <Runtime/Containers/Singleton.h>

#include <string>
#include <unordered_map>

namespace Horizon
{
	class IAssetImporter;

	struct ImporterTypeInfo
	{
		std::string_view defaultName;
		std::string_view extension;
		u32 version;
		IAssetImporter* (*CreateImporter)();
	};

	class ImporterRegistry : public Singleton<ImporterRegistry>
	{
	public:
		void Register(const ImporterTypeInfo& info)
		{
			m_byExtension[ToLower(info.extension)] = info;
			Terminal::Log("ImporterRegistry", "Importer registered for {}", info.extension);
		}

		const ImporterTypeInfo* Find(std::string_view extension) const
		{
			auto it = m_byExtension.find(ToLower(extension));
			if (it == m_byExtension.end())
				return nullptr;

			return &it->second;
		}

	private:
		static std::string ToLower(std::string_view s)
		{
			std::string out(s);
			for (char& c : out)
				c = (char)std::tolower((unsigned char)c);

			return out;
		}

	private:
		std::unordered_map<std::string, ImporterTypeInfo> m_byExtension;
	};
}