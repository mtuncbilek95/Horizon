#pragma once

namespace Horizon
{

	enum class VertexInputRate
	{
		Vertex = 0,
		Instance = 1
	};

	enum class PrimitiveTopology
	{
		PointList = 0,
		LineList = 1,
		LineStrip = 2,
		TriangleList = 3,
		TriangleStrip = 4,
		TriangleFan = 5,
		LineListAdj = 6,
		LineStripAdj = 7,
		TriangleListAdj = 8,
		TriangleStripAdj = 9
	};

	enum class DynamicState
	{
		Viewport = 1 << 0,
		Scissor = 1 << 1,
		PolygonMode = 1 << 2,
		PrimitiveTopology = 1 << 3,
	};

	enum class PolygonMode
	{
		Fill = 0,
		Line = 1,
		Point = 2
	};

	enum class CullMode
	{
		None = 0,
		Front = 1,
		Back = 2,
		FrontAndBack = 3,
	};

	enum class FaceOrientation
	{
		CW = 0,
		CCW = 1
	};

	enum class BlendFactor
	{
		Zero = 0,
		One = 1,
		SrcColor = 2,
		OneMinusSrcColor = 3,
		DstColor = 4,
		OneMinusDstColor = 5,
		SrcAlpha = 6,
		OneMinusSrcAlpha = 7,
		DstAlpha = 8,
		OneMinusDstAlpha = 9,
		ConstantColor = 10,
		OneMinusConstantColor = 11,
		ConstantAlpha = 12,
		OneMinusConstantAlpha = 13,
		Src1Color = 14,
		OneMinusSrc1Color = 15,
		Src1Alpha = 16,
		OneMinusSrc1Alpha = 17
	};

	enum class BlendOp
	{
		Add = 0,
		Subtract = 1,
		ReverseSubtract = 2,
		Min = 3,
		Max
	};

	enum class CompareOp
	{
		Never = 0,
		Less = 1,
		Equal = 2,
		LesserEqual = 3,
		Greater = 4,
		NotEqual = 5,
		GreaterEqual = 6,
		Always = 7
	};

	enum class ColorComponent
	{
		Red = 1 << 0,
		Green = 1 << 1,
		Blue = 1 << 2,
		Alpha = 1 << 3,
		All = 0x3FFF
	};

	enum class LogicOp
	{
		Clear = 0,
		And = 1,
		AndReverse = 2,
		Copy = 3,
		AndInverted = 4,
		NoOp = 5,
		Xor = 6,
		Or = 7,
		Nor = 8,
		Equivalent = 9,
		Invert = 10,
		OrReverse = 11,
		CopyInverted = 12,
		OrInverted = 13,
		Nand = 14,
		Set = 15
	};

	enum class PipelineType
	{
		Graphics,
		Compute,
		Raytracing
	};

	enum class PipelineFlags
	{
		None = 0,
		DescriptorBuffer = 1 << 0,
	};
}
