#include "DX12Backend.h"

#include <Engine/Graphics/RHI/GfxIndirect.h>

namespace Horizon
{
	using namespace DX12;

	GfxCommandSignatureHandle GfxDevice::CreateCommandSignature(const GfxCommandSignatureDesc& desc)
	{
		Context& context = GfxContext();

		D3D12_INDIRECT_ARGUMENT_DESC args[2] = {};
		u32 argCount = 0;
		u32 stride = 0;

		if (desc.constantCount > 0)
		{
			args[argCount].Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT;
			args[argCount].Constant.RootParameterIndex = 0;
			args[argCount].Constant.DestOffsetIn32BitValues = 0;
			args[argCount].Constant.Num32BitValuesToSet = desc.constantCount;
			argCount++;
			stride += desc.constantCount * 4;
		}

		switch (desc.type)
		{
		case GfxIndirectType::Draw:
			args[argCount].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW;
			stride += sizeof(D3D12_DRAW_ARGUMENTS);
			break;
		case GfxIndirectType::DrawIndexed:
			args[argCount].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;
			stride += sizeof(D3D12_DRAW_INDEXED_ARGUMENTS);
			break;
		case GfxIndirectType::Dispatch:
			args[argCount].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH;
			stride += sizeof(D3D12_DISPATCH_ARGUMENTS);
			break;
		}
		argCount++;

		D3D12_COMMAND_SIGNATURE_DESC commandSigDesc = {};
		commandSigDesc.ByteStride = stride;
		commandSigDesc.NumArgumentDescs = argCount;
		commandSigDesc.pArgumentDescs = args;

		ID3D12RootSignature* rootSig = (desc.constantCount > 0) ? context.rootSignature : nullptr;

		ID3D12CommandSignature* signature = nullptr;
		context.device->CreateCommandSignature(&commandSigDesc, rootSig, IID_PPV_ARGS(&signature));

		SetDebugName(signature, desc.debugName);

		u32 index;
		if (!context.cmdSigFreeList.empty())
		{
			index = context.cmdSigFreeList.back();
			context.cmdSigFreeList.pop_back();
		}
		else
		{
			index = (u32)context.cmdSigs.size();
			context.cmdSigs.push_back(nullptr);
			context.cmdSigStrides.push_back(0);
			context.cmdSigGenerations.push_back(0);
		}

		context.cmdSigs[index] = signature;
		context.cmdSigStrides[index] = stride;
		return GfxCommandSignatureHandle::make(index, context.cmdSigGenerations[index]);
	}

	void GfxDevice::DestroyCommandSignature(GfxCommandSignatureHandle handle)
	{
		Context& context = GfxContext();
		if (context.cmdSigs[handle.index()])
			context.cmdSigs[handle.index()]->Release();
		context.cmdSigs[handle.index()] = nullptr;
		context.cmdSigGenerations[handle.index()]++;
		context.cmdSigFreeList.push_back(handle.index());
	}

	u32 GfxDevice::CommandSignatureStride(GfxCommandSignatureHandle handle)
	{
		return GfxContext().cmdSigStrides[handle.index()];
	}
}
