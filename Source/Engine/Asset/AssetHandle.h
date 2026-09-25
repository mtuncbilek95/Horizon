#pragma once

#include <Engine/Asset/AssetObject.h>
#include <Runtime/Containers/Guid.h>

#include <type_traits>

namespace Horizon::Engine
{
	template<typename T>
		requires std::is_base_of_v<AssetObject, T>
	class H_EXPORT AssetHandle
	{
	public:
		AssetHandle() = default;
		AssetHandle(const Guid& id, T* pObject) : m_id(id), m_object(pObject)
		{
		}
		~AssetHandle() = default;

		const Guid& GetId() const { return m_id; }
		T* GetAsset() const { return m_object; }

		void SetId(const Guid& id) { m_id = id; }
		void SetAsset(AssetObject* pObj) { m_object = (T*)pObj; }

	private:
		Guid m_id;
		T* m_object = nullptr;
	};
}