#pragma once

#include <Runtime/Containers/Guid.h>
#include <Runtime/Containers/List.h>
#include <Runtime/RTTR/Reflection.h>

#include <string>

namespace Horizon::Editor
{
	class DomainFolder;
	class DomainFile;
	class DomainService;

	class EDITOR_API AssetPickerWidget
	{
		struct Candidate
		{
			DomainFile* pFile = nullptr;
			Guid id;
			std::string label;
			std::string tooltip;
		};

	public:
		AssetPickerWidget() = default;
		~AssetPickerWidget() = default;

		b8 Draw(Guid* pId, Reflect::Type* pAssetType, DomainService* pDomain);

	private:
		b8 DrawDropTarget(Guid* pId, Reflect::Type* pAssetType);
		b8 DrawPicker(Guid* pId, DomainService* pDomain, Reflect::Type* pAssetType);

		void RefreshCandidates(DomainService* pDomain, Reflect::Type* pAssetType);
		void CollectProducts(DomainFolder* pFolder, Reflect::Type* pAssetType);
		void CollectProducts(DomainFile* pFile, Reflect::Type* pAssetType, List<Candidate>& outCandidates) const;

	private:
		std::string m_search;
		List<Candidate> m_candidates;
		std::string m_candidateTypeName;
		u64 m_candidateRevision = 0;
		b8 m_openPickerRequest = false;
	};
}