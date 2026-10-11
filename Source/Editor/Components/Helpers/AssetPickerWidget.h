#pragma once

#include <Runtime/Containers/Guid.h>
#include <Runtime/Containers/List.h>

#include <string>

namespace Horizon::Editor
{
	class DomainFolder;
	class DomainFile;
	class DomainService;

	class EDITOR_API AssetPickerWidget
	{
	public:
		AssetPickerWidget() = default;
		~AssetPickerWidget() = default;

		b8 Draw(Guid* pId, const std::string& assetTypeName, DomainService* pDomain);

	private:
		b8 DrawDropTarget(Guid* pId, const std::string& assetTypeName);
		b8 DrawPicker(Guid* pId, DomainService* pDomain, const std::string& assetTypeName);

		void RefreshCandidates(DomainService* pDomain, const std::string& assetTypeName);
		void CollectFiles(DomainFolder* pFolder, const std::string& assetTypeName);

	private:
		std::string m_search;
		List<DomainFile*> m_candidates;
		std::string m_candidateTypeName;
		u64 m_candidateRevision = 0;
	};
}