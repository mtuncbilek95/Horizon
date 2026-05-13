#include "VkPipelineUtils.h"

#include <magic_enum/magic_enum.hpp>

namespace Horizon
{
	VkCompareOp VkPipelineUtils::GetVkCompareOp(CompareOp op)
	{
		switch (op)
		{
		case CompareOp::Never:
			return VK_COMPARE_OP_NEVER;
		case CompareOp::Less:
			return VK_COMPARE_OP_LESS;
		case CompareOp::Equal:
			return VK_COMPARE_OP_EQUAL;
		case CompareOp::LesserEqual:
			return VK_COMPARE_OP_LESS_OR_EQUAL;
		case CompareOp::Greater:
			return VK_COMPARE_OP_GREATER;
		case CompareOp::NotEqual:
			return VK_COMPARE_OP_NOT_EQUAL;
		case CompareOp::GreaterEqual:
			return VK_COMPARE_OP_GREATER_OR_EQUAL;
		case CompareOp::Always:
		default:
			return VK_COMPARE_OP_ALWAYS;
		}
	}

	VkPipelineBindPoint VkPipelineUtils::GetVkBindPoint(PipelineType type)
	{
		switch (type)
		{
		case PipelineType::Graphics:
			return VK_PIPELINE_BIND_POINT_GRAPHICS;
		case PipelineType::Compute:
			return VK_PIPELINE_BIND_POINT_COMPUTE;
		case PipelineType::Raytracing:
			return VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR;
		default:
			return VK_PIPELINE_BIND_POINT_GRAPHICS;
		}
	}

	VkImageLayout VkPipelineUtils::GetVkImgLayout(ImageLayout layout)
	{
		switch (layout)
		{
		case ImageLayout::Undefined:
			return VK_IMAGE_LAYOUT_UNDEFINED;
		case ImageLayout::General:
			return VK_IMAGE_LAYOUT_GENERAL;
		case ImageLayout::ColorAttachmentOptimal:
			return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		case ImageLayout::DepthStencilAttachmentOptimal:
			return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		case ImageLayout::DepthStencilReadOnlyOptimal:
			return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
		case ImageLayout::ShaderReadOnlyOptimal:
			return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		case ImageLayout::TransferSrcOptimal:
			return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		case ImageLayout::TransferDstOptimal:
			return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		case ImageLayout::Preinitialized:
			return VK_IMAGE_LAYOUT_PREINITIALIZED;
		case ImageLayout::PresentSrcKHR:
			return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		default:
			return VK_IMAGE_LAYOUT_UNDEFINED;
		}
	}

	VkAttachmentStoreOp VkPipelineUtils::GetVkStoreOp(AttachmentStore op)
	{
		switch (op)
		{
		case AttachmentStore::DontCare:
			return VK_ATTACHMENT_STORE_OP_DONT_CARE;
		case AttachmentStore::Store:
			return VK_ATTACHMENT_STORE_OP_STORE;
		default:
			return VK_ATTACHMENT_STORE_OP_DONT_CARE;
		}
	}

	VkAttachmentLoadOp VkPipelineUtils::GetVkLoadOp(AttachmentLoad op)
	{
		switch (op)
		{
		case AttachmentLoad::Load:
			return VK_ATTACHMENT_LOAD_OP_LOAD;
		case AttachmentLoad::Clear:
			return VK_ATTACHMENT_LOAD_OP_CLEAR;
		case AttachmentLoad::DontCare:
			return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		default:
			return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		}
	}

	VkShaderStageFlags VkPipelineUtils::GetShaderType(ShaderStage stage)
	{
		VkShaderStageFlags flags = 0;

		if (HasFlag(stage, ShaderStage::Vertex))
			flags |= VK_SHADER_STAGE_VERTEX_BIT;
		if (HasFlag(stage, ShaderStage::TessControl))
			flags |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
		if (HasFlag(stage, ShaderStage::TessEval))
			flags |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
		if (HasFlag(stage, ShaderStage::Geometry))
			flags |= VK_SHADER_STAGE_GEOMETRY_BIT;
		if (HasFlag(stage, ShaderStage::Fragment))
			flags |= VK_SHADER_STAGE_FRAGMENT_BIT;
		if (HasFlag(stage, ShaderStage::Compute))
			flags |= VK_SHADER_STAGE_COMPUTE_BIT;
		if (HasFlag(stage, ShaderStage::Raygen))
			flags |= VK_SHADER_STAGE_RAYGEN_BIT_KHR;
		if (HasFlag(stage, ShaderStage::Anyhit))
			flags |= VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
		if (HasFlag(stage, ShaderStage::ClosestHit))
			flags |= VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
		if (HasFlag(stage, ShaderStage::Miss))
			flags |= VK_SHADER_STAGE_MISS_BIT_KHR;
		if (HasFlag(stage, ShaderStage::Intersection))
			flags |= VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
		if (HasFlag(stage, ShaderStage::Callable))
			flags |= VK_SHADER_STAGE_CALLABLE_BIT_KHR;
		if (HasFlag(stage, ShaderStage::Task))
			flags |= VK_SHADER_STAGE_TASK_BIT_EXT;
		if (HasFlag(stage, ShaderStage::Mesh))
			flags |= VK_SHADER_STAGE_MESH_BIT_EXT;

		return flags;
	}

	VkShaderStageFlagBits VkPipelineUtils::GetShaderTypeBit(ShaderStage stage)
	{
		switch (stage)
		{
		case ShaderStage::Vertex:
			return VK_SHADER_STAGE_VERTEX_BIT;
		case ShaderStage::TessControl:
			return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
		case ShaderStage::TessEval:
			return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
		case ShaderStage::Geometry:
			return VK_SHADER_STAGE_GEOMETRY_BIT;
		case ShaderStage::Fragment:
			return VK_SHADER_STAGE_FRAGMENT_BIT;
		case ShaderStage::Compute:
			return VK_SHADER_STAGE_COMPUTE_BIT;
		case ShaderStage::Raygen:
			return VK_SHADER_STAGE_RAYGEN_BIT_KHR;
		case ShaderStage::Anyhit:
			return VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
		case ShaderStage::ClosestHit:
			return VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
		case ShaderStage::Miss:
			return VK_SHADER_STAGE_MISS_BIT_KHR;
		case ShaderStage::Intersection:
			return VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
		case ShaderStage::Callable:
			return VK_SHADER_STAGE_CALLABLE_BIT_KHR;
		case ShaderStage::Task:
			return VK_SHADER_STAGE_TASK_BIT_EXT;
		case ShaderStage::Mesh:
			return VK_SHADER_STAGE_MESH_BIT_EXT;
		default:
			return VK_SHADER_STAGE_VERTEX_BIT;
		}
	}

	VkPrimitiveTopology VkPipelineUtils::GetVkTopo(PrimitiveTopology topo)
	{
		switch (topo)
		{
		case PrimitiveTopology::PointList:
			return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
		case PrimitiveTopology::LineList:
			return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
		case PrimitiveTopology::LineStrip:
			return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
		case PrimitiveTopology::TriangleList:
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		case PrimitiveTopology::TriangleStrip:
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		case PrimitiveTopology::TriangleFan:
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
		case PrimitiveTopology::LineListAdj:
			return VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;
		case PrimitiveTopology::LineStripAdj:
			return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY;
		case PrimitiveTopology::TriangleListAdj:
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY;
		case PrimitiveTopology::TriangleStripAdj:
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY;
		default:
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		}
	}

	VkPolygonMode VkPipelineUtils::GetVkPolygonMode(PolygonMode mode)
	{
		switch (mode)
		{
		case PolygonMode::Fill:
			return VK_POLYGON_MODE_FILL;
		case PolygonMode::Line:
			return VK_POLYGON_MODE_LINE;
		case PolygonMode::Point:
			return VK_POLYGON_MODE_POINT;
		default:
			return VK_POLYGON_MODE_FILL;
		}
	}

	VkCullModeFlags VkPipelineUtils::GetVkCullMode(CullMode mode)
	{
		switch (mode)
		{
		case CullMode::Zero:
			return VK_CULL_MODE_NONE;
		case CullMode::Front:
			return VK_CULL_MODE_FRONT_BIT;
		case CullMode::Back:
			return VK_CULL_MODE_BACK_BIT;
		case CullMode::FrontAndBack:
			return VK_CULL_MODE_FRONT_AND_BACK;
		default:
			return VK_CULL_MODE_NONE;
		}
	}

	VkFrontFace VkPipelineUtils::GetVkFrontFace(FaceOrientation frontFace)
	{
		switch (frontFace)
		{
		case FaceOrientation::CW:
			return VK_FRONT_FACE_CLOCKWISE;
		case FaceOrientation::CCW:
			return VK_FRONT_FACE_COUNTER_CLOCKWISE;
		default:
			return VK_FRONT_FACE_COUNTER_CLOCKWISE;
		}
	}

	VkBlendFactor VkPipelineUtils::GetVkBlendFactor(BlendFactor factor)
	{
		switch (factor)
		{
		case BlendFactor::Zero:
			return VK_BLEND_FACTOR_ZERO;
		case BlendFactor::One:
			return VK_BLEND_FACTOR_ONE;
		case BlendFactor::SrcColor:
			return VK_BLEND_FACTOR_SRC_COLOR;
		case BlendFactor::OneMinusSrcColor:
			return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
		case BlendFactor::DstColor:
			return VK_BLEND_FACTOR_DST_COLOR;
		case BlendFactor::OneMinusDstColor:
			return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
		case BlendFactor::SrcAlpha:
			return VK_BLEND_FACTOR_SRC_ALPHA;
		case BlendFactor::OneMinusSrcAlpha:
			return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		case BlendFactor::DstAlpha:
			return VK_BLEND_FACTOR_DST_ALPHA;
		case BlendFactor::OneMinusDstAlpha:
			return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
		case BlendFactor::ConstantColor:
			return VK_BLEND_FACTOR_CONSTANT_COLOR;
		case BlendFactor::OneMinusConstantColor:
			return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
		case BlendFactor::ConstantAlpha:
			return VK_BLEND_FACTOR_CONSTANT_ALPHA;
		case BlendFactor::OneMinusConstantAlpha:
			return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
		default:
			return VK_BLEND_FACTOR_ZERO;
		}
	}

	VkBlendOp VkPipelineUtils::GetVkBlendOp(BlendOp op)
	{
		switch (op)
		{
		case BlendOp::Add:
			return VK_BLEND_OP_ADD;
		case BlendOp::Subtract:
			return VK_BLEND_OP_SUBTRACT;
		case BlendOp::ReverseSubtract:
			return VK_BLEND_OP_REVERSE_SUBTRACT;
		case BlendOp::Min:
			return VK_BLEND_OP_MIN;
		case BlendOp::Max:
			return VK_BLEND_OP_MAX;
		default:
			return VK_BLEND_OP_ADD;
		}
	}

	VkColorComponentFlags VkPipelineUtils::GetVkColorComponents(ColorComponent components)
	{
		VkColorComponentFlags flags = 0;
		for (auto s : magic_enum::enum_values<ColorComponent>())
		{
			if (IsSameFlag(s, ColorComponent::All))
				return VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

			if (HasFlag(components, ColorComponent::Red))
				flags |= VK_COLOR_COMPONENT_R_BIT;
			if (HasFlag(components, ColorComponent::Green))
				flags |= VK_COLOR_COMPONENT_G_BIT;
			if (HasFlag(components, ColorComponent::Blue))
				flags |= VK_COLOR_COMPONENT_B_BIT;
			if (HasFlag(components, ColorComponent::Alpha))
				flags |= VK_COLOR_COMPONENT_A_BIT;
		}

		return flags;
	}

	VkVertexInputRate VkPipelineUtils::GetVkVertexInputRate(VertexInputRate rate)
	{
		switch (rate)
		{
		case VertexInputRate::Vertex:
			return VK_VERTEX_INPUT_RATE_VERTEX;
		case VertexInputRate::Instance:
			return VK_VERTEX_INPUT_RATE_INSTANCE;
		default:
			return VK_VERTEX_INPUT_RATE_VERTEX;
		}
	}
	VkLogicOp VkPipelineUtils::GetVkLogicOp(LogicOp op)
	{
		switch (op)
		{
		case LogicOp::Clear:
			return VK_LOGIC_OP_CLEAR;
		case LogicOp::And:
			return VK_LOGIC_OP_AND;
		case LogicOp::AndReverse:
			return VK_LOGIC_OP_AND_REVERSE;
		case LogicOp::Copy:
			return VK_LOGIC_OP_COPY;
		case LogicOp::AndInverted:
			return VK_LOGIC_OP_AND_INVERTED;
		case LogicOp::NoOp:
			return VK_LOGIC_OP_NO_OP;
		case LogicOp::Xor:
			return VK_LOGIC_OP_XOR;
		case LogicOp::Or:
			return VK_LOGIC_OP_OR;
		case LogicOp::Nor:
			return VK_LOGIC_OP_NOR;
		case LogicOp::Equivalent:
			return VK_LOGIC_OP_EQUIVALENT;
		case LogicOp::Invert:
			return VK_LOGIC_OP_INVERT;
		case LogicOp::OrReverse:
			return VK_LOGIC_OP_OR_REVERSE;
		case LogicOp::CopyInverted:
			return VK_LOGIC_OP_COPY_INVERTED;
		case LogicOp::OrInverted:
			return VK_LOGIC_OP_OR_INVERTED;
		case LogicOp::Nand:
			return VK_LOGIC_OP_NAND;
		case LogicOp::Set:
			return VK_LOGIC_OP_SET;
		default:
			return VK_LOGIC_OP_NO_OP;
		}
	}

	VkDynamicState VkPipelineUtils::GetVkDynamicState(DynamicState state)
	{
		switch (state)
		{
		case DynamicState::Viewport:
			return VK_DYNAMIC_STATE_VIEWPORT;
		case DynamicState::Scissor:
			return VK_DYNAMIC_STATE_SCISSOR;
		case DynamicState::CullMode:
			return VK_DYNAMIC_STATE_CULL_MODE;
		case DynamicState::FrontFace:
			return VK_DYNAMIC_STATE_FRONT_FACE;
		case DynamicState::PrimitiveTopology:
			return VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY;
		case DynamicState::DepthTestEnable:
			return VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE;
		case DynamicState::DepthWriteEnable:
			return VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE;
		case DynamicState::DepthCompareOp:
			return VK_DYNAMIC_STATE_DEPTH_COMPARE_OP;
		case DynamicState::DepthBoundsTestEnable:
			return VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE;
		case DynamicState::DepthBounds:
			return VK_DYNAMIC_STATE_DEPTH_BOUNDS;
		case DynamicState::DepthBiasEnable:
			return VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE;
		case DynamicState::PolygonMode:
			return VK_DYNAMIC_STATE_POLYGON_MODE_EXT;
		case DynamicState::LogicOpEnable:
			return VK_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT;
		case DynamicState::LogicOp:
			return VK_DYNAMIC_STATE_LOGIC_OP_EXT;
		case DynamicState::ColorBlendEnable:
			return VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT;
		case DynamicState::ColorBlendEquation:
			return VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT;
		case DynamicState::ColorWriteMask:
			return VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT;
		default:
			return VK_DYNAMIC_STATE_VIEWPORT;
		}
	}

	VkPipelineCreateFlags VkPipelineUtils::GetVkPipelineFlags(PipelineFlags flags)
	{
		VkPipelineCreateFlags vkFlags = 0;

		if (HasFlag(flags, PipelineFlags::DescriptorBuffer))
			vkFlags |= VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;

		return vkFlags;
	}

	VkPipelineStageFlags VkPipelineUtils::GetVkPipelineStageFlags(PipelineStageFlags flags)
	{
		if (flags == PipelineStageFlags::Auto)
			return 0;

		VkPipelineStageFlags vkFlags = 0;

		for (usize i = 0; i <= 18; ++i)
		{
			PipelineStageFlags val = static_cast<PipelineStageFlags>(1ull << i);

			if (HasFlag(flags, val))
			{
				switch (val)
				{
				case PipelineStageFlags::TopOfPipe:
					vkFlags |= VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
					break;
				case PipelineStageFlags::DrawIndirect:
					vkFlags |= VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;
					break;
				case PipelineStageFlags::VertexInput:
					vkFlags |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
					break;
				case PipelineStageFlags::VertexShader:
					vkFlags |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
					break;
				case PipelineStageFlags::TessControlShader:
					vkFlags |= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT;
					break;
				case PipelineStageFlags::TessEvalShader:
					vkFlags |= VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
					break;
				case PipelineStageFlags::GeometryShader:
					vkFlags |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
					break;
				case PipelineStageFlags::FragmentShader:
					vkFlags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
					break;
				case PipelineStageFlags::EarlyFragment:
					vkFlags |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
					break;
				case PipelineStageFlags::LateFragment:
					vkFlags |= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
					break;
				case PipelineStageFlags::ColorAttachment:
					vkFlags |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
					break;
				case PipelineStageFlags::ComputeShader:
					vkFlags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
					break;
				case PipelineStageFlags::Transfer:
					vkFlags |= VK_PIPELINE_STAGE_TRANSFER_BIT;
					break;
				case PipelineStageFlags::BottomOfPipe:
					vkFlags |= VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
					break;
				case PipelineStageFlags::Host:
					vkFlags |= VK_PIPELINE_STAGE_HOST_BIT;
					break;
				case PipelineStageFlags::AccelerationStructure:
					vkFlags |= VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
					break;
				case PipelineStageFlags::RaytracingShader:
					vkFlags |= VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;
					break;
				case PipelineStageFlags::TaskShader:
					vkFlags |= VK_PIPELINE_STAGE_TASK_SHADER_BIT_EXT;
					break;
				case PipelineStageFlags::MeshShader:
					vkFlags |= VK_PIPELINE_STAGE_MESH_SHADER_BIT_EXT;
					break;
				default:
					break;
				}
			}
		}

		return vkFlags;
	}

	VkAccessFlags VkPipelineUtils::GetVkAccessFlags(AccessFlags flags)
	{
		if (flags == AccessFlags::Auto)
			return 0;

		VkAccessFlags vkFlags = 0;

		for (usize i = 0; i <= 16; ++i)
		{
			AccessFlags val = static_cast<AccessFlags>(1ull << i);

			if (HasFlag(flags, val))
			{
				switch (val)
				{
				case AccessFlags::IndirectCommandRead:
					vkFlags |= VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
					break;
				case AccessFlags::IndexRead:
					vkFlags |= VK_ACCESS_INDEX_READ_BIT;
					break;
				case AccessFlags::VertexAttributeRead:
					vkFlags |= VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
					break;
				case AccessFlags::UniformRead:
					vkFlags |= VK_ACCESS_UNIFORM_READ_BIT;
					break;
				case AccessFlags::InputAttachmentRead:
					vkFlags |= VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
					break;
				case AccessFlags::ShaderRead:
					vkFlags |= VK_ACCESS_SHADER_READ_BIT;
					break;
				case AccessFlags::ShaderWrite:
					vkFlags |= VK_ACCESS_SHADER_WRITE_BIT;
					break;
				case AccessFlags::ColorAttachmentRead:
					vkFlags |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
					break;
				case AccessFlags::ColorAttachmentWrite:
					vkFlags |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
					break;
				case AccessFlags::DepthStencilAttachmentRead:
					vkFlags |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
					break;
				case AccessFlags::DepthStencilAttachmentWrite:
					vkFlags |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
					break;
				case AccessFlags::TransferRead:
					vkFlags |= VK_ACCESS_TRANSFER_READ_BIT;
					break;
				case AccessFlags::TransferWrite:
					vkFlags |= VK_ACCESS_TRANSFER_WRITE_BIT;
					break;
				case AccessFlags::HostRead:
					vkFlags |= VK_ACCESS_HOST_READ_BIT;
					break;
				case AccessFlags::HostWrite:
					vkFlags |= VK_ACCESS_HOST_WRITE_BIT;
					break;
				case AccessFlags::MemoryRead:
					vkFlags |= VK_ACCESS_MEMORY_READ_BIT;
					break;
				case AccessFlags::MemoryWrite:
					vkFlags |= VK_ACCESS_MEMORY_WRITE_BIT;
					break;
				default:
					break;
				}
			}
		}

		return vkFlags;
	}
}
