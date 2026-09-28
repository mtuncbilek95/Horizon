#include "FileView.h"

namespace Horizon::PAL
{
	FileView::FileView() : m_pBase(nullptr), m_pData(nullptr), m_size(0)
	{
	}

	FileView::FileView(void* pBase, const u8* pData, usize size) : m_pBase(pBase), m_pData(pData), m_size(size)
	{
	}

	void FileView::Release()
	{
		m_pBase = nullptr;
		m_pData = nullptr;
		m_size = 0;
	}
}