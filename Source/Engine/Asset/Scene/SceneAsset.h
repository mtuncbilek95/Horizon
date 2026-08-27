#pragma once

#include <Engine/Asset/Asset.h>
#include <Engine/Asset/Scene/SceneChunk.h>
#include <Runtime/Containers/Guid.h>
#include <Runtime/Containers/List.h>

#include <string>

namespace Horizon::Engine
{
	HCLASS();
	class H_EXPORT SceneAsset final : public Asset
	{
		HORIZON_TYPE_REFLECT(SceneAsset);
		friend class SceneLoadStrategy;
	public:
		SceneAsset() = default;
		~SceneAsset() = default;

		u32 GetEntityCount() const { return m_entityCount; }
		const List<SceneChunkDesc>& GetChunks() const { return m_chunks; }
		const List<std::string>& GetTypeNames() const { return m_typeNames; }
		const List<Guid>& GetDependencies() const { return m_dependencies; }
		const List<u8>& GetBlob() const { return m_blob; }

	private:
		u32 m_entityCount = 0;
		List<SceneChunkDesc> m_chunks;
		List<std::string> m_typeNames;
		List<Guid> m_dependencies;
		List<u8> m_blob;
	};
}