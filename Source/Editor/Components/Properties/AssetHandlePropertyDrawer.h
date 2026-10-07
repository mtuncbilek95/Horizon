#pragma once

#include <Editor/Components/PropertyDrawer.h>
#include <Runtime/Containers/Guid.h>
#include <Runtime/Containers/List.h>

#include <string>

namespace Horizon::Editor
{
	class DomainFolder;
	class DomainFile;
	class DomainService;

	HCLASS();
	class EDITOR_API AssetHandlePropertyDrawer final : public PropertyDrawer
	{
		HORIZON_TYPE_REFLECT(AssetHandlePropertyDrawer);
	public:
		AssetHandlePropertyDrawer() = default;
		~AssetHandlePropertyDrawer() = default;

		Reflect::TypeHandle GetTargetType() const final { return Reflect::TypeOf<Guid>(); }
		b8 OnDraw(const Reflect::Field& field, void* pValue, const PropertyContext& ctx) final;

	private:
		b8 DrawDropTarget(const Reflect::Field& field, Guid* pId, const std::string& assetTypeName);
		b8 DrawPicker(Guid* pId, DomainService* pDomain, const std::string& assetTypeName);

		void RefreshCandidates(DomainService* pDomain, const std::string& assetTypeName);
		void CollectFiles(DomainFolder* pFolder, const std::string& assetTypeName);

		static b8 MatchesSearch(const std::string& name, const std::string& search);

	private:
		std::string m_search;
		List<DomainFile*> m_candidates;
		std::string m_candidateTypeName;
		u64 m_candidateRevision = 0;
	};
}