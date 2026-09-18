#pragma once

#include <Engine/Asset/AssetResidency.h>
#include <Engine/Core/Engine.h>
#include <Runtime/Containers/Guid.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	class H_EXPORT AssetStreamer : public Reflect::Base
	{
		friend class AssetService;
	public:
		Engine* GetEngine() const { return m_engine; }

		virtual void OnInitialize() = 0;
		virtual void OnFinalize() = 0;

		virtual Reflect::TypeHandle GetAssetType() = 0;

	private:
		Engine* m_engine;
	};
}