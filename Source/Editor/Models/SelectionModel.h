#pragma once

#include <Runtime/Definitions/Handle.h>
#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/RTTR/TypeHandle.h>

namespace Horizon::Editor
{
	class H_EXPORT SelectionModel final
	{
	public:
		SelectionModel() = default;
		~SelectionModel() = default;

		SelectionModel(const SelectionModel&) = delete;
		SelectionModel& operator=(const SelectionModel&) = delete;

		template<typename Tag>
		void Set(HandleEx<Tag> handle)
		{
			m_domain = Reflect::TypeOf<Tag>();
			m_id = handle.index;
		}

		template<typename Tag>
		b8 Is() const { return m_domain == Reflect::TypeOf<Tag>() && m_id != InvalidHandleId; }

		template<typename Tag>
		HandleEx<Tag> Get() const
		{
			HandleEx<Tag> handle;

			if (m_domain != Reflect::TypeOf<Tag>())
				return handle;

			handle.index = m_id;
			return handle;
		}

		void Clear()
		{
			m_domain = Reflect::TypeHandle();
			m_id = InvalidHandleId;
		}

		b8 IsEmpty() const { return m_id == InvalidHandleId; }
		Reflect::TypeHandle GetDomain() const { return m_domain; }

	private:
		Reflect::TypeHandle m_domain;
		HandleId m_id = InvalidHandleId;
	};
}