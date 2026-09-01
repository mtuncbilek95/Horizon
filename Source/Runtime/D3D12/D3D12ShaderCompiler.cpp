#include <Runtime/RHI/Shader/GfxShaderCompiler.h>

#include <Runtime/Log/Terminal.h>

#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif

#if !defined(NOMINMAX)
#define NOMINMAX
#endif

#include <Windows.h>
#include <unknwn.h>
#include <objidl.h>
#include <wrl/client.h>
#include <dxcapi.h>

using Microsoft::WRL::ComPtr;

namespace Horizon::RHI
{
	namespace
	{
		std::wstring ToWide(const std::string& s)
		{
			const i32 len = MultiByteToWideChar(CP_UTF8, 0, s.data(), (i32)s.size(), nullptr, 0);
			std::wstring out(len, L'\0');
			MultiByteToWideChar(CP_UTF8, 0, s.data(), (i32)s.size(), out.data(), len);
			return out;
		}

		std::wstring ToShaderModel(GfxShaderStage stage)
		{
			switch (stage)
			{
			case GfxShaderStage::Vertex:			return L"vs_6_6";
			case GfxShaderStage::TessCtrl:			return L"hs_6_6";
			case GfxShaderStage::TessEval:			return L"ds_6_6";
			case GfxShaderStage::Geometry:			return L"gs_6_6";
			case GfxShaderStage::Pixel:				return L"ps_6_6";
			case GfxShaderStage::Compute:			return L"cs_6_6";
			case GfxShaderStage::Task:				return L"as_6_6";
			case GfxShaderStage::Mesh:				return L"ms_6_6";
			case GfxShaderStage::RayGeneration:
			case GfxShaderStage::Miss:
			case GfxShaderStage::ClosestHit:
			case GfxShaderStage::AnyHit:
			case GfxShaderStage::Intersection:
			case GfxShaderStage::Callable:
				return L"lib_6_6";
			default:
				Terminal::Error("D3D12ShaderCompiler", "Shader stage {} has no target profile", u32(stage));
				return std::wstring();
			}
		}
	}

	List<u8> GfxShaderCompiler::Compile(const std::string& filePath, GfxShaderStage stage, const std::string& entryPoint, const std::string& includePath)
	{
		ComPtr<IDxcUtils> utils;
		ComPtr<IDxcCompiler3> compiler;
		DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils));
		DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));

		ComPtr<IDxcIncludeHandler> includeHandler;
		utils->CreateDefaultIncludeHandler(&includeHandler);

		const std::wstring wPath = ToWide(filePath);
		const std::wstring wSM = ToShaderModel(stage);
		const std::wstring wEntry = ToWide(entryPoint);
		const std::wstring wInclude = ToWide(includePath);

		ComPtr<IDxcBlobEncoding> source;
		if (FAILED(utils->LoadFile(wPath.c_str(), nullptr, &source)))
			return List<u8>();

		DxcBuffer sourceBuffer{};
		sourceBuffer.Ptr = source->GetBufferPointer();
		sourceBuffer.Size = source->GetBufferSize();
		sourceBuffer.Encoding = DXC_CP_ACP;

		List<std::wstring> arguments =
		{
			wPath.c_str(), L"-E", wEntry.c_str(), L"-T", wSM.c_str(),
			 L"-HV", L"2021"
		};

		if (!includePath.empty())
		{
			arguments.PushBack(L"-I");
			arguments.PushBack(wInclude);
		}

#if defined(HORIZON_DEBUG)
		arguments.PushBack(L"-Zi");
		arguments.PushBack(L"-Qembed_debug");
		arguments.PushBack(L"-Od");
#else
		arguments.PushBack(L"-O3");
#endif

		List<const wchar_t*> argPointers;
		argPointers.Reserve(arguments.GetCount());

		for (const std::wstring& argument : arguments)
			argPointers.PushBack(argument.c_str());

		ComPtr<IDxcResult> result;
		HRESULT hr = compiler->Compile(&sourceBuffer, argPointers.GetData(), 
			(u32)argPointers.GetCount(), includeHandler.Get(), IID_PPV_ARGS(&result));

		if (FAILED(hr))
			return List<u8>();

		ComPtr<IDxcBlobUtf8> errors;
		result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr);
		if (errors && errors->GetStringLength() > 0)
		{
			Terminal::Error("D3D12ShaderCompiler", "{}", errors->GetStringPointer());
			return List<u8>();
		}

		HRESULT status = S_OK;
		result->GetStatus(&status);
		if (FAILED(status))
			return List<u8>();

		ComPtr<IDxcBlob> object;
		result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&object), nullptr);
		if (!object)
			return List<u8>();

		const u8* begin = static_cast<const u8*>(object->GetBufferPointer());
		return List<u8>(begin, begin + object->GetBufferSize());
	}
}