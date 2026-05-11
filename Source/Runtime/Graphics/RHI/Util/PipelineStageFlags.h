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
        MeshShader = 1 << 18,
		Auto = 1 << 31
    };

	enum class AccessFlags : usize
	{
		None = 0,
		IndirectCommandRead = 1 << 0,
		IndexRead = 1 << 1,
		VertexAttributeRead = 1 << 2,
		UniformRead = 1 << 3,
		InputAttachmentRead = 1 << 4,
		ShaderRead = 1 << 5,
		ShaderWrite = 1 << 6,
		ColorAttachmentRead = 1 << 7,
		ColorAttachmentWrite = 1 << 8,
		DepthStencilAttachmentRead = 1 << 9,
		DepthStencilAttachmentWrite = 1 << 10,
		TransferRead = 1 << 11,
		TransferWrite = 1 << 12,
		HostRead = 1 << 13,
		HostWrite = 1 << 14,
		MemoryRead = 1 << 15,
		MemoryWrite = 1 << 16,
		Auto = 1 << 31
	};
}
