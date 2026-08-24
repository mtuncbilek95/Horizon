#pragma once

#include <Runtime/RHI/Upload/GfxUploadRing.h>
#include <Runtime/D3D12/D3D12Helpers.h>

#include <Runtime/Containers/List.h>

namespace Horizon::RHI
{
	class D3D12Device;

	class D3D12UploadRing final : public GfxUploadRing
	{
		friend class D3D12Device;
	public:
		~D3D12UploadRing() final;

		GfxUploadRange Allocate(usize size, usize alignment) final;
		void MarkFrame(GfxFence* pFence, u64 fenceValue) final;

	private:
		struct Marker
		{
			u64 head = 0;
			u64 fenceValue = 0;
		};

		b8 RetireOldest();

		u8* m_mapped = nullptr;

		u64 m_head = 0;
		u64 m_tail = 0;

		List<Marker> m_markers;
		GfxFence* m_fence = nullptr;
	};
}
