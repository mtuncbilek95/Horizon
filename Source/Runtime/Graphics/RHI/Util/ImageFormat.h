#pragma once

namespace Horizon
{
    enum class ImageViewType
    {
        View1D,
        View2D,
        View3D,
        ViewCube,
        View1DArray,
        View2DArray,
        View3DArray,
        ViewCubeArray
    };

    enum class ImageAspect
    {
        Color,
        Depth,
        Stencil,
        Meta
    };

    enum class ImageType
    {
        Image1D,
        Image2D,
        Image3D
    };

    enum class SampleCount
    {
        Sample1,
        Sample2,
        Sample4,
        Sample8,
        Sample16,
        Sample32,
        Sample64
    };

    enum class ImageLayout
    {
        Undefined,
        General,
        ColorAttachmentOptimal,
		DepthStencilAttachmentOptimal,
        DepthStencilReadOnlyOptimal,
        ShaderReadOnlyOptimal,
        TransferSrcOptimal,
        TransferDstOptimal,
        Preinitialized,
        PresentSrcKHR
    };

    enum class AttachmentStore
    {
        DontCare,
        Store
    };

    enum class AttachmentLoad
    {
        Load,
        Clear,
        DontCare
	};

    enum class ImageUsage
    {
        TransferSrc = 1 << 0,
        TransferDst = 1 << 1,
        Sampled = 1 << 2,
        Storage = 1 << 3,
        ColorAttachment = 1 << 4,
        DepthStencil = 1 << 5,
        TransientAttachment = 1 << 6,
        InputAttachment = 1 << 7,
        HostTransfer = 1 << 8,
        VideoEncodeDst = 1 << 9,
        VideoEncodeSrc = 1 << 10,
        VideoEncodeDpb = 1 << 11,
        VideoDecodeDst = 1 << 12,
        VideoDecodeSrc = 1 << 13,
        VideoDecodeDpb = 1 << 14
    };

    enum class ImageFormat
    {
        Undefined = 0,

        R8_UNorm,
        R8_SNorm,
        R8_UInt,
        R8_SInt,
        R8_SRGB,

        R8G8_UNorm,
        R8G8_SNorm,
        R8G8_UInt,
        R8G8_SInt,
        R8G8_SRGB,

        R8G8B8_UNorm,
        R8G8B8_SNorm,
        R8G8B8_UInt,
        R8G8B8_SInt,
        R8G8B8_SRGB,

        R8G8B8A8_UNorm,
        R8G8B8A8_SNorm,
        R8G8B8A8_UInt,
        R8G8B8A8_SInt,
        R8G8B8A8_SRGB,

        R16_SFloat,
        R16G16_SFloat,
        R16G16B16_SFloat,
        R16G16B16A16_SFloat,

        R32_SFloat,
        R32G32_SFloat,
        R32G32B32_SFloat,
        R32G32B32A32_SFloat,

        D16_UNorm,
        D32_SFloat,
        S8_UInt,
        D24_UNorm_S8_UInt,
        D32_SFloat_S8_UInt,
    };
}
