#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::PAL
{
	class H_EXPORT FileView final
	{
		friend struct File;
	public:
		FileView();
		FileView(void* pBase, const u8* pData, usize size);
		~FileView() = default;

		b8 IsValid() const { return m_data != nullptr; }
		const u8* GetData() const { return m_data; }
		usize GetSize() const { return m_size; }

	private:
		void Release();

	private:
		void* m_base;
		const u8* m_data;
		usize m_size;
	};
}