#include "ShaderCompiler.h"

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
		std::wstring ToWide(const std::string& value)
		{
			const i32 length = MultiByteToWideChar(CP_UTF8, 0, value.data(), static_cast<i32>(value.size()), nullptr, 0);
			std::wstring result(length, L'\0');
			MultiByteToWideChar(CP_UTF8, 0, value.data(), static_cast<i32>(value.size()), result.data(), length);
			return result;
		}

		std::wstring ToProfile(ShaderStage stage)
		{
			switch (stage)
			{
			case ShaderStage::Vertex: return L"vs_6_6";
			case ShaderStage::TessCtrl: return L"hs_6_6";
			case ShaderStage::TessEval: return L"ds_6_6";
			case ShaderStage::Geometry: return L"gs_6_6";
			case ShaderStage::Pixel: return L"ps_6_6";
			case ShaderStage::Compute: return L"cs_6_6";
			case ShaderStage::Task: return L"as_6_6";
			case ShaderStage::Mesh: return L"ms_6_6";
			case ShaderStage::RayGeneration:
			case ShaderStage::Miss:
			case ShaderStage::ClosestHit:
			case ShaderStage::AnyHit:
			case ShaderStage::Intersection:
			case ShaderStage::Callable:
				return L"lib_6_6";
			default:
				Terminal::Error("ShaderCompiler", "Shader stage {} has no target profile", static_cast<u32>(stage));
				return std::wstring();
			}
		}
	}

	List<u8> ShaderCompiler::Compile(const ShaderCompileDesc& desc)
	{
		std::wstring profile = ToProfile(desc.stage);
		if (profile.empty())
			return List<u8>();

		ComPtr<IDxcUtils> utils;
		ComPtr<IDxcCompiler3> compiler;
		DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils));
		DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));

		ComPtr<IDxcIncludeHandler> includeHandler;
		utils->CreateDefaultIncludeHandler(&includeHandler);

		std::wstring path = ToWide(desc.filePath);

		ComPtr<IDxcBlobEncoding> source;
		if (FAILED(utils->LoadFile(path.c_str(), nullptr, &source)))
		{
			Terminal::Error("ShaderCompiler", "Failed to load shader: {}", desc.filePath);
			return List<u8>();
		}

		DxcBuffer sourceBuffer = {};
		sourceBuffer.Ptr = source->GetBufferPointer();
		sourceBuffer.Size = source->GetBufferSize();
		sourceBuffer.Encoding = DXC_CP_ACP;

		List<std::wstring> arguments = { path, L"-E", ToWide(desc.entryPoint), L"-T", profile, L"-HV", L"2021" };

		for (const std::string& includePath : desc.includePaths)
		{
			arguments.PushBack(L"-I");
			arguments.PushBack(ToWide(includePath));
		}

		for (const std::string& define : desc.defines)
		{
			arguments.PushBack(L"-D");
			arguments.PushBack(ToWide(define));
		}

		if (desc.target == ShaderTarget::Vulkan)
		{
			arguments.PushBack(L"-spirv");
			arguments.PushBack(L"-fspv-target-env=vulkan1.3");
			arguments.PushBack(L"-fvk-use-dx-layout");
			arguments.PushBack(L"-fvk-bind-resource-heap");
			arguments.PushBack(L"0");
			arguments.PushBack(L"0");
			arguments.PushBack(L"-fvk-bind-sampler-heap");
			arguments.PushBack(L"1");
			arguments.PushBack(L"0");
		}

#if defined(HORIZON_DEBUG)
		arguments.PushBack(L"-Od");
		arguments.PushBack(L"-Zi");

		if (desc.target == ShaderTarget::Vulkan)
			arguments.PushBack(L"-fspv-debug=vulkan-with-source");
		else
			arguments.PushBack(L"-Qembed_debug");
#elif defined(HORIZON_RELEASE)
		arguments.PushBack(L"-O3");

		if (desc.target == ShaderTarget::Vulkan)
		{
			arguments.PushBack(L"-fspv-reduce-load-size");
		}
		else
		{
			arguments.PushBack(L"-all-resources-bound");
			arguments.PushBack(L"-Qstrip_debug");
			arguments.PushBack(L"-Qstrip_reflect");
		}
#else
		arguments.PushBack(L"-O3");
#endif

		List<const wchar_t*> argPointers;
		argPointers.Reserve(arguments.GetCount());

		for (const std::wstring& argument : arguments)
			argPointers.PushBack(argument.c_str());

		ComPtr<IDxcResult> result;
		HRESULT hr = compiler->Compile(&sourceBuffer, argPointers.GetData(),
			static_cast<u32>(argPointers.GetCount()), includeHandler.Get(), IID_PPV_ARGS(&result));
		if (FAILED(hr))
		{
			Terminal::Error("ShaderCompiler", "Compile call failed: {}", desc.filePath);
			return List<u8>();
		}

		ComPtr<IDxcBlobUtf8> errors;
		result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr);

		HRESULT status = S_OK;
		result->GetStatus(&status);
		if (FAILED(status))
		{
			Terminal::Error("ShaderCompiler", "{}: {}", desc.filePath, errors ? errors->GetStringPointer() : "Unknown error");
			return List<u8>();
		}

		if (errors && errors->GetStringLength() > 0)
			Terminal::Warn("ShaderCompiler", "{}: {}", desc.filePath, errors->GetStringPointer());

		ComPtr<IDxcBlob> object;
		result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&object), nullptr);
		if (!object)
		{
			Terminal::Error("ShaderCompiler", "No object output: {}", desc.filePath);
			return List<u8>();
		}

		const u8* pBegin = static_cast<const u8*>(object->GetBufferPointer());
		return List<u8>(pBegin, pBegin + object->GetBufferSize());
	}
}