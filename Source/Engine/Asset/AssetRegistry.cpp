#include "AssetRegistry.h"

#include <filesystem>
#include <fstream>

namespace Horizon
{
	constexpr u32 kMagic = 0x48415247;
	constexpr u32 kVersion = 1;

	template<typename T> 
	void Write(std::ostream& os, const T& v)
	{
		os.write(reinterpret_cast<const char*>(&v), sizeof(T));
	}

	template<typename T> 
	void Read(std::istream& is, T& v)
	{
		is.read(reinterpret_cast<char*>(&v), sizeof(T));
	}

	void WriteString(std::ostream& os, const std::string& s)
	{
		Write<u32>(os, static_cast<u32>(s.size()));
		os.write(s.data(), static_cast<std::streamsize>(s.size()));
	}

	void ReadString(std::istream& is, std::string& s)
	{
		u32 len = 0; Read(is, len);
		s.resize(len);
		is.read(s.data(), static_cast<std::streamsize>(len));
	}

	b8 AssetRegistry::SaveToFile(const std::string& path) const
	{
		std::ofstream os(path, std::ios::binary);
		if (!os) 
			return false;

		std::shared_lock lock(m_mutex);
		Write(os, kMagic);
		Write(os, kVersion);
		Write<u32>(os, static_cast<u32>(m_entries.size()));

		for (const auto& [guid, meta] : m_entries)
		{
			Write(os, meta.guid);
			Write(os, meta.type);
			WriteString(os, meta.cookedPath);
			Write<u32>(os, static_cast<u32>(meta.dependencies.size()));

			for (const Guid& dep : meta.dependencies)
				Write(os, dep);
		}

		return static_cast<b8>(os);
	}

	b8 AssetRegistry::LoadFromFile(const std::string& path)
	{
		std::filesystem::path fullPath = path;
		std::filesystem::path dir = fullPath.parent_path();

		if (!dir.empty() && !std::filesystem::exists(dir))
			std::filesystem::create_directories(dir);

		std::ofstream file(path);
		file.close();

		std::ifstream is(path, std::ios::binary);
		if (!is) 
			return false;

		u32 magic = 0, version = 0, count = 0;
		Read(is, magic);
		Read(is, version);

		if (magic != kMagic || version != kVersion) 
			return false;

		Read(is, count);

		std::unique_lock lock(m_mutex);
		m_entries.clear();
		m_entries.reserve(count);

		for (u32 i = 0; i < count; ++i)
		{
			AssetMetadata meta;
			Read(is, meta.guid);
			Read(is, meta.type);
			ReadString(is, meta.cookedPath);

			u32 depCount = 0; Read(is, depCount);
			meta.dependencies.resize(depCount);
			for (u32 d = 0; d < depCount; ++d)
				Read(is, meta.dependencies[d]);

			m_entries.emplace(meta.guid, std::move(meta));
		}

		return static_cast<b8>(is);
	}
}