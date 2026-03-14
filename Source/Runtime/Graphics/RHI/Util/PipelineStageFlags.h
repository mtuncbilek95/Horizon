#pragma once

namespace Horizon
{
    enum class PipelineStageFlags : usize
    {
        TopOfPipe = 1 << 0,
        DrawIndirect = 1 << 1,
        VertexInput = 1 << 2,
        VertexShader = 1 << 3,
        TessControlShader = 1 << 4,
        TessEvalShader = 1 << 5,
        GeometryShader = 1 << 6,
        FragmentShader = 1 << 7,
        EarlyFragment = 1 << 8,
        LateFragment = 1 << 9,
        ColorAttachment = 1 << 10,
        ComputeShader = 1 << 11,
        Transfer = 1 << 12,
        BottomOfPipe = 1 << 13,
        Host = 1 << 14,
        AccelerationStructure = 1 << 15,
        RaytracingShader = 1 << 16,
        TaskShader = 1 << 17,
        MeshShader = 1 << 18
    };
}
