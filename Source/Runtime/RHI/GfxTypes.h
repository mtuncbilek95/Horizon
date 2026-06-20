#pragma once

namespace Horizon
{
	inline constexpr u32 MaxFramesInFlight = 2;

	enum class GfxDescriptorHeapType : u8
	{
		Resource,
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
		R8, RG8, RGB8, RGBA8, RGBA8_sRGB,
		BGRA8, BGRA8_sRGB,
		R16F, RG16F, RGBA16F,
		R32F, RG32F, RGBA32F,
		R11G11B10F, RGB10A2,
		R32U, RG32U,
		D16, D32, D24S8,
		BC7, BC7_sRGB
	};

	enum class GfxTextureType : u8
	{
		Tex2D,
		Tex2DArray,
		Tex3D
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
		GPU,
		CPU,
		HostVisible
	};

	enum class GfxBufferUsage : u8
	{
		None = 0,
		Vertex = 1 << 0,
		Index = 1 << 1,
		Constant = 1 << 2,
		Storage = 1 << 3,
		Indirect = 1 << 4,
		CopySrc = 1 << 5,
		CopyDst = 1 << 6
	};

	enum class GfxFilter : u8
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
		Geometry = 1 << 1,
		Pixel = 1 << 2,
		Compute = 1 << 3,
		Mesh = 1 << 4,
		Task = 1 << 5,
		All = 0x7FFFFFFF
	};

	enum class GfxPipelineType : u8
	{
		Graphics,
		Compute
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
		Undefined,
		Common,
		VertexBuffer,
		IndexBuffer,
		ConstantBuffer,
		IndirectArg,
		ShaderResource,
		UnorderedAccess,
		RenderTarget,
		DepthWrite,
		DepthRead,
		CopySrc,
		CopyDst,
		Present
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
}