#include "ShaderAsset.h"

#include <Runtime/ShaderCompiler/ShaderCompiler.h>

namespace Horizon
{
    void ShaderAsset::Import()
    {
        ShaderData data = ShaderCompiler::ReadShaderData(m_physicalPath);
        m_stage = data.stage;
        m_entryPoint = data.entryPoint;
        m_spirv = ShaderCompiler::GenerateSpirv(data.sourceText, data.entryPoint, data.stage);
        m_imported = true;

        Log::Terminal(LogType::Success, "ShaderAsset", "Imported {}", m_virtualPath);
    }

    void ShaderAsset::Release()
    {
        m_spirv = ReadArray<u32>();
        m_imported = false;
    }
}
