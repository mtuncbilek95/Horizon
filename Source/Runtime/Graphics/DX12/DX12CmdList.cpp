#include "DX12Context.h"

namespace Horizon
{
	void GfxCmdList::UploadBuffer(GfxCmdListHandle command, GfxBufferHandle src, usize srcOffset, GfxBufferHandle dst, usize dstOffset, usize sizeInBytes)
	{
		Context& ctx = DX12Context();
		ID3D12Resource* pSrc = nullptr;
		ID3D12Resource* pDst = nullptr;

		ctx.bufferPool.ResolveRead(src, [&](const DX12Buffer& b) 
			{ 
				pSrc = b.pResource; 
			});

		ctx.bufferPool.ResolveRead(dst, [&](const DX12Buffer& b) 
			{ 
				pDst = b.pResource; 
			});

		ResolveCmdList(command, [&](DX12CmdList& c) {
			c.pList->CopyBufferRegion(pDst, dstOffset, pSrc, srcOffset, sizeInBytes);
			});
	}
}