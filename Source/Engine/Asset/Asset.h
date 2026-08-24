#pragma once

#include <Runtime/Containers/Guid.h>
#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	HCLASS();
	class H_EXPORT Asset : public Reflect::Base
	{
		friend class AssetService;
	public:
		Asset() = default;
		virtual ~Asset() = default;

		Asset(const Asset&) = delete;
		Asset& operator=(const Asset&) = delete;

		const Guid& GetID() const { return m_guid; }

	private:
		Guid m_guid;
	};
}