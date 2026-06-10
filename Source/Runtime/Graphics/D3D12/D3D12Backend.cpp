#include "D3D12Backend.h"

namespace Horizon
{
	void Helpers::CreateTerminalLog(GfxDevice* pContext)
	{
		if (SUCCEEDED(pContext->pDevice->QueryInterface(IID_PPV_ARGS(&pContext->pInfoQueue))))
		{
			auto callback = [](D3D12_MESSAGE_CATEGORY category, D3D12_MESSAGE_SEVERITY severity,
				D3D12_MESSAGE_ID messageId, LPCSTR desc, void* ctx)
				{
					std::string cString;
					std::string sString;

					switch (category)
					{
					case D3D12_MESSAGE_CATEGORY_APPLICATION_DEFINED:
						cString = "APPLICATION-DEFINED";
						break;
					case D3D12_MESSAGE_CATEGORY_MISCELLANEOUS:
						cString = "MISCELLANEOUS";
						break;
					case D3D12_MESSAGE_CATEGORY_INITIALIZATION:
						cString = "INITIALIZATION";
						break;
					case D3D12_MESSAGE_CATEGORY_CLEANUP:
						cString = "CLEANUP";
						break;
					case D3D12_MESSAGE_CATEGORY_COMPILATION:
						cString = "COMPILATION";
						break;
					case D3D12_MESSAGE_CATEGORY_STATE_CREATION:
						cString = "STATE-CREATION";
						break;
					case D3D12_MESSAGE_CATEGORY_STATE_SETTING:
						cString = "STATE-SETTING";
						break;
					case D3D12_MESSAGE_CATEGORY_STATE_GETTING:
						cString = "STATE-GETTING";
						break;
					case D3D12_MESSAGE_CATEGORY_RESOURCE_MANIPULATION:
						cString = "RESOURCE-MANIPULATION";
						break;
					case D3D12_MESSAGE_CATEGORY_EXECUTION:
						cString = "EXECUTION";
						break;
					case D3D12_MESSAGE_CATEGORY_SHADER:
						cString = "SHADER";
						break;
					default:
						cString = "UNKNOWN";
						break;
					}

					switch (severity)
					{
					case D3D12_MESSAGE_SEVERITY_CORRUPTION:
						sString = "CORRUPTION";
						break;
					case D3D12_MESSAGE_SEVERITY_ERROR:
						sString = "ERROR";
						break;
					case D3D12_MESSAGE_SEVERITY_WARNING:
						sString = "WARNING";
						break;
					case D3D12_MESSAGE_SEVERITY_INFO:
						sString = "INFO";
						break;
					case D3D12_MESSAGE_SEVERITY_MESSAGE:
						sString = "MESSAGE";
						break;
					}

					MainLog::Terminal("[{}][{}]: {}", cString, sString, desc);
				};

			pContext->pInfoQueue->RegisterMessageCallback(callback, D3D12_MESSAGE_CALLBACK_FLAG_NONE, nullptr, &pContext->pInfoId);
		}
	}
}