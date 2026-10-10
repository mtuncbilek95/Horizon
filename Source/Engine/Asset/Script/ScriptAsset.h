#pragma once

#include <Engine/Asset/AssetObject.h>
#include <Engine/Job/JobSystem.h>

namespace Horizon::Engine
{
	HCLASS();
	class ENGINE_API ScriptAsset : public AssetObject
	{
		HORIZON_TYPE_REFLECT(ScriptAsset);
		friend class ScriptAssetStreamer;
	public:
		ScriptAsset();
		~ScriptAsset();

		void LoadAsync();
		void UnloadAsync();

	private:
		SubmitTicket m_ticket = InvalidSubmitTicket;
	};
}