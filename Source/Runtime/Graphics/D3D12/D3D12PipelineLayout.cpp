#include "D3D12Backend.h"

#include <Runtime/Graphics/GfxBackend.h>

namespace Horizon
{
	GfxPipelineLayout* Gfx::CreateGfxGlobalPipelineLayout(GfxDevice* pDevice)
	{
		constexpr u32 kRootConstantCount = 16;

		D3D12_ROOT_PARAMETER1 params[1] = {};
		params[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		params[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		params[0].Constants.ShaderRegister = 0;
		params[0].Constants.RegisterSpace = 0;
		params[0].Constants.Num32BitValues = kRootConstantCount;

		auto makeSampler = [](u32 reg, D3D12_FILTER filter, D3D12_TEXTURE_ADDRESS_MODE addr,
			D3D12_COMPARISON_FUNC comp = D3D12_COMPARISON_FUNC_NEVER,
			D3D12_STATIC_BORDER_COLOR border = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK,
			u32 aniso = 0)
			{
				D3D12_STATIC_SAMPLER_DESC sampDesc = {};
				sampDesc.Filter = filter;
				sampDesc.AddressU = addr;
				sampDesc.AddressV = addr;
				sampDesc.AddressW = addr;
				sampDesc.MipLODBias = 0.0f;
				sampDesc.MaxAnisotropy = aniso;
				sampDesc.ComparisonFunc = comp;
				sampDesc.BorderColor = border;
				sampDesc.MinLOD = 0.0f;
				sampDesc.MaxLOD = D3D12_FLOAT32_MAX;
				sampDesc.ShaderRegister = reg;
				sampDesc.RegisterSpace = 0;
				sampDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
				return sampDesc;
			};

		const D3D12_STATIC_SAMPLER_DESC samplers[] =
		{
			makeSampler(0, D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_WRAP),					// PointWrap
			makeSampler(1, D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP),					// PointClamp
			makeSampler(2, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP),					// LinearWrap
			makeSampler(3, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP),					// LinearClamp
			makeSampler(4, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_MIRROR),					// LinearMirror
			makeSampler(5, D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_WRAP,
						   D3D12_COMPARISON_FUNC_NEVER, D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK, 16),			// AnisoWrap
			makeSampler(6, D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
						   D3D12_COMPARISON_FUNC_NEVER, D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK, 16),			// AnisoClamp
			makeSampler(7, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_BORDER),					// LinearBorder
			makeSampler(8, D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
						   D3D12_COMPARISON_FUNC_LESS_EQUAL, D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE),			// ShadowCmp
			makeSampler(9, D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_BORDER),					// PointBorder
		};

		D3D12_VERSIONED_ROOT_SIGNATURE_DESC vdesc = {};
		vdesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
		vdesc.Desc_1_1.NumParameters = _countof(params);
		vdesc.Desc_1_1.pParameters = params;
		vdesc.Desc_1_1.NumStaticSamplers = _countof(samplers);
		vdesc.Desc_1_1.pStaticSamplers = samplers;
		vdesc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED;

		ID3DBlob* pBlob = nullptr;
		ID3DBlob* pError = nullptr;
		HRESULT bResult = D3D12SerializeVersionedRootSignature(&vdesc, &pBlob, &pError);
		CHECK_HR(bResult, "ID3D12RootSignature - D3D12SerializeVersionedRootSignature");

		GfxPipelineLayout* pLayout = new GfxPipelineLayout();
		bResult = pDevice->pDevice->CreateRootSignature(0, pBlob->GetBufferPointer(), pBlob->GetBufferSize(),
			IID_PPV_ARGS(&pLayout->pLayout));
		CHECK_HR(bResult, "ID3D12RootSignature - CreateRootSignature");

		pBlob->Release();

		return pLayout;
	}

	void Gfx::DestroyGfxPipelineLayout(GfxPipelineLayout* layoutHandl)
	{
		if (layoutHandl->pLayout)
			layoutHandl->pLayout->Release();

		delete layoutHandl;
	}
}