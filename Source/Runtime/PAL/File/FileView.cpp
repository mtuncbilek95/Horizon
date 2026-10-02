#include "FileView.h"

namespace Horizon::PAL
{
	FileView::FileView() : m_base(nullptr), m_data(nullptr), m_size(0)
	{
	}

	FileView::FileView(void* pBase, const u8* pData, usize size) : m_base(pBase), m_data(pData), m_size(size)
	{
	}

	void FileView::Release()
	{
		m_base = nullptr;
		m_data = nullptr;
		m_size = 0;
	}
}