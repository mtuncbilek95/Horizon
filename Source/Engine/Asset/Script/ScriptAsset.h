#pragma once

#include <Engine/Asset/AssetObject.h>
#include <Engine/Asset/Script/ScriptField.h>
#include <Engine/Job/JobSystem.h>
#include <Runtime/Containers/List.h>

#include <string>

namespace Horizon::Engine
{
	HCLASS();
	class ENGINE_API ScriptAsset : public AssetObject
	{
		HORIZON_TYPE_REFLECT(ScriptAsset);
		friend class ScriptAssetStreamer;
	public:
		ScriptAsset() = default;
		~ScriptAsset() = default;

		void LoadAsync();
		void UnloadAsync();

		const std::string& GetSource() const { return m_source; }
		const std::string& GetChunkName() const { return m_chunkName; }
		const List<ScriptField>& GetFields() const { return m_fields; }

	private:
		SubmitTicket m_ticket = InvalidSubmitTicket;

		std::string m_source;
		std::string m_chunkName;
		List<ScriptField> m_fields;
	};
}