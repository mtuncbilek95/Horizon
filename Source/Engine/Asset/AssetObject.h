#pragma once

#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	class AssetStreamer;

	class H_EXPORT AssetObject : public Reflect::Base
	{
	public:
		virtual ~AssetObject() = default;

		template<typename T>
		T* GetStreamer() const { return (T*)GetStreamer(); }
		AssetStreamer* GetStreamer() const { return m_streamer; }

	private:
		AssetStreamer* m_streamer = nullptr;
	};
}