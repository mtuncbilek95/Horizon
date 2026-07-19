#pragma once

#include <Runtime/Definitions/BitwiseOperators.h>
#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon
{
	enum class GfxDescriptorHeapType : u8
	{
		Resource,
		Sampler,
		Color,
		Depth
	};

	enum class GfxQueueType : u8
	{
		Graphics,
		Compute,
		Transfer,
		Count
	};

	enum class GfxTextureFormat : u8
	{
		Undefined,

		R8, RG8, RGBA8, RGBA8_sRGB,
		BGRA8, BGRA8_sRGB,
		R8_SNORM, RG8_SNORM,
		R16F, RG16F, RGBA16F,
		R32F, RG32F, RGBA32F,
		R11G11B10F, RGB10A2, RGB9E5,
		R8U, R16U,
		R32U, RG32U,
		D16, D32, D24S8, D32S8,
		BC1, BC1_sRGB,
		BC3, BC3_sRGB,
		BC4,
		BC5,
		BC6H,
		BC7, BC7_sRGB
	};

	enum class GfxTextureType : u8
	{
		Tex1D,
		Tex2D,
		Tex3D
	};

	enum class GfxTextureTypeFlags : u8
	{
		None = 0,
		Array = 1 << 0,
		Cube = 1 << 1
	};

	enum class GfxTextureUsage : u32
	{
		None = 0,
		Sampled = 1 << 0,
		RenderTarget = 1 << 1,
		DepthStencil = 1 << 2,
		Storage = 1 << 3,
		TransferSrc = 1 << 4,
		TransferDst = 1 << 5
	};

	enum class GfxMemoryType : u8
	{
		GpuOnly,
		Upload,
		Readback
	};

	enum class GfxBufferUsage : u32
	{
		None = 0,
		Vertex = 1 << 0,
		Index = 1 << 1,
		Constant = 1 << 2,
		Storage = 1 << 3,
		Indirect = 1 << 4,
		CopySrc = 1 << 5,
		CopyDst = 1 << 6,
		AccelerationStructure = 1 << 7,
		ShaderDeviceAddress = 1 << 8
	};

	enum class GfxSampleCount : u8
	{
		Count1 = 1,
		Count2 = 2,
		Count4 = 4,
		Count8 = 8
	};

	enum class GfxFilter : u8
	{
		Nearest,
		Linear
	};

	enum class GfxMipFilter : u8
	{
		Nearest,
		Linear
	};

	enum class GfxAddressMode : u8
	{
		Repeat,
		MirrorRepeat,
		ClampEdge,
		ClampBorder
	};

	enum class GfxBorderColor : u8
	{
		TransparentBlack,
		OpaqueBlack,
		OpaqueWhite
	};

	enum class GfxShaderStage : u32
	{
		None = 0,

		Vertex = 1 << 0,
		TessCtrl = 1 << 1,
		TessEval = 1 << 2,
		Geometry = 1 << 3,
		Pixel = 1 << 4,

		Compute = 1 << 5,

		Task = 1 << 6,
		Mesh = 1 << 7,

		RayGeneration = 1 << 8,
		Miss = 1 << 9,
		ClosestHit = 1 << 10,
		AnyHit = 1 << 11,
		Intersection = 1 << 12,
		Callable = 1 << 13,

		AllGraphics = Vertex | TessCtrl | TessEval | Geometry | Pixel,
		AllMesh = Task | Mesh,
		AllRayTracing = RayGeneration | Miss | ClosestHit | AnyHit | Intersection | Callable,
		All = AllGraphics | Compute | AllMesh | AllRayTracing
	};

	enum class GfxPipelineType : u8
	{
		Graphics,
		Compute,
		Raytracing
	};

	enum class GfxPrimitiveTopology : u8
	{
		PointList,
		LineList,
		TriangleList,
		TriangleStrip
	};

	enum class GfxFillMode : u8
	{
		Solid,
		Wireframe
	};

	enum class GfxCullMode : u8
	{
		None,
		Front,
		Back
	};

	enum class GfxFrontFace : u8
	{
		CW,
		CCW
	};

	enum class GfxCompareOp : u8
	{
		Never, Less, Equal, LessEqual,
		Greater, NotEqual, GreaterEqual, Always
	};

	enum class GfxBlendFactor : u8
	{
		Zero, One,
		SrcColor, InvSrcColor,
		SrcAlpha, InvSrcAlpha,
		DstColor, InvDstColor,
		DstAlpha, InvDstAlpha
	};

	enum class GfxBlendOp : u8
	{
		Add, Subtract, RevSubtract, Min, Max
	};

	enum class GfxColorWrite : u8
	{
		None = 0,
		R = 1 << 0, G = 1 << 1, B = 1 << 2, A = 1 << 3,
		All = R | G | B | A
	};

	enum class GfxStencilOp : u8
	{
		Keep, Zero, Replace,
		IncrSat, DecrSat, Invert,
		IncrWrap, DecrWrap
	};

	enum class GfxResourceState : u32
	{
		Undefined = 0,
		Common = 1 << 0,
		VertexBuffer = 1 << 1,
		IndexBuffer = 1 << 2,
		ConstantBuffer = 1 << 3,
		IndirectArg = 1 << 4,
		ShaderResource = 1 << 5,
		UnorderedAccess = 1 << 6,
		RenderTarget = 1 << 7,
		DepthWrite = 1 << 8,
		DepthRead = 1 << 9,
		CopySrc = 1 << 10,
		CopyDst = 1 << 11,
		ResolveSrc = 1 << 12,
		ResolveDst = 1 << 13,
		AccelerationStructure = 1 << 14,
		Present = 1 << 15,
	};

	enum class GfxLoadOp : u8
	{
		Load,
		Clear,
		DontCare
	};

	enum class GfxStoreOp : u8
	{
		Store,
		DontCare
	};

	enum class GfxIndexType : u8
	{
		Index16,
		Index32
	};

	enum class GfxStaticSampler : u8
	{
		PointClamp,
		PointWrap,
		LinearClamp,
		LinearWrap,
		LinearMirror,
		AnisoClamp,
		AnisoWrap,
		ShadowCompare,
		RevShadowCompare,
		Count
	};

	struct GfxSamplerDesc
	{
		GfxFilter minFilter = GfxFilter::Linear;
		GfxFilter magFilter = GfxFilter::Linear;
		GfxMipFilter mipFilter = GfxMipFilter::Linear;
		GfxAddressMode addressU = GfxAddressMode::Repeat;
		GfxAddressMode addressV = GfxAddressMode::Repeat;
		GfxAddressMode addressW = GfxAddressMode::Repeat;
		GfxBorderColor borderColor = GfxBorderColor::OpaqueBlack;
		b8 anisotropyEnable = false;
		u8 maxAnisotropy = 1;
		b8 compareEnable = false;
		GfxCompareOp compareOp = GfxCompareOp::Never;
		f32 mipLodBias = 0.0f;
		f32 minLod = 0.0f;
		f32 maxLod = 1000.0f;
	};

	struct GfxBlendState
	{
		b8 enable = false;
		GfxBlendFactor srcColor = GfxBlendFactor::One;
		GfxBlendFactor dstColor = GfxBlendFactor::Zero;
		GfxBlendOp colorOp = GfxBlendOp::Add;
		GfxBlendFactor srcAlpha = GfxBlendFactor::One;
		GfxBlendFactor dstAlpha = GfxBlendFactor::Zero;
		GfxBlendOp alphaOp = GfxBlendOp::Add;
		GfxColorWrite writeMask = GfxColorWrite::All;
	};

	struct GfxColor
	{
		f32 r, g, b, a;
	};

	struct GfxViewport
	{
		f32 x = 0.0f;
		f32 y = 0.0f;
		f32 width = 0.0f;
		f32 height = 0.0f;
		f32 minDepth = 0.0f;
		f32 maxDepth = 1.0f;
	};

	struct GfxScissor
	{
		i32 x = 0;
		i32 y = 0;
		i32 width = 0;
		i32 height = 0;
	};
}