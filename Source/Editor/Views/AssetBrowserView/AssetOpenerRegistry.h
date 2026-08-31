#pragma once

#include <Editor/Views/AssetBrowserView/AssetOpener.h>
#include <Runtime/Containers/List.h>

#include <string>
#include <string_view>

namespace Horizon::Engine
{
	class Engine;
}

namespace Horizon::Editor
{
	class DomainFile;

	class H_EXPORT AssetOpenerRegistry final
	{
		struct OpenerEntry
		{
			AssetOpener* pOpener = nullptr;
			std::string assetTypeName;
		};

	public:
		AssetOpenerRegistry() = default;
		~AssetOpenerRegistry();

		AssetOpenerRegistry(const AssetOpenerRegistry&) = delete;
		AssetOpenerRegistry& operator=(const AssetOpenerRegistry&) = delete;

		void Bootstrap(Engine::Engine* pEngine);
		void Clear();

		b8 CanOpen(std::string_view assetTypeName) const;
		b8 Open(Engine::Engine* pEngine, DomainFile* pFile) const;

	private:
		AssetOpener* Find(std::string_view assetTypeName) const;

		List<OpenerEntry> m_openers;
	};
}