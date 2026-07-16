#pragma once

namespace Horizon
{
	class IAsset
	{
		friend class AssetSystem;

	public:
		virtual ~IAsset() = default;

		const Guid& GetGuid() const { return m_guid; }

	private:
		void SetGuid(const Guid& guid) { m_guid = guid; }

	private:
		Guid m_guid;
	};
}