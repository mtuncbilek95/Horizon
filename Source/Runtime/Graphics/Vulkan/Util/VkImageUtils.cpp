#include "VkImageUtils.h"

#include <magic_enum/magic_enum.hpp>

#include <unordered_map>

namespace Horizon
{
	std::unordered_map<VkFormat, u32> MakeFormatSizeMap()
	{
		std::unordered_map<VkFormat, u32> map;
		for (auto format : magic_enum::enum_values<VkFormat>())
		{
			u32 size = 0;
			if (format == VK_FORMAT_R4G4_UNORM_PACK8 || format == VK_FORMAT_S8_UINT) size = 1;
			else if (format >= VK_FORMAT_R4G4B4A4_UNORM_PACK16 && format <= VK_FORMAT_A1R5G5B5_UNORM_PACK16)
				size = 2;
			else if (format >= VK_FORMAT_R8_UNORM && format <= VK_FORMAT_R8_SRGB) size = 1;
			else if (format >= VK_FORMAT_R8G8_UNORM && format <= VK_FORMAT_R8G8_SRGB) size = 2;
			else if (format >= VK_FORMAT_R8G8B8_UNORM && format <= VK_FORMAT_B8G8R8_SRGB) size = 3;
			else if (format >= VK_FORMAT_R8G8B8A8_UNORM && format <= VK_FORMAT_A2B10G10R10_SINT_PACK32)
				size = 4;
			else if (format >= VK_FORMAT_R16_UNORM && format <= VK_FORMAT_R16_SFLOAT) size = 2;
			else if (format >= VK_FORMAT_R16G16_UNORM && format <= VK_FORMAT_R16G16_SFLOAT) size = 4;
			else if (format >= VK_FORMAT_R16G16B16_UNORM && format <= VK_FORMAT_R16G16B16_SFLOAT) size = 6;
			else if (format >= VK_FORMAT_R16G16B16A16_UNORM && format <= VK_FORMAT_R16G16B16A16_SFLOAT)
				size = 8;
			else if (format >= VK_FORMAT_R32_UINT && format <= VK_FORMAT_R32_SFLOAT) size = 4;
			else if (format >= VK_FORMAT_R32G32_UINT && format <= VK_FORMAT_R32G32_SFLOAT) size = 8;
			else if (format >= VK_FORMAT_R32G32B32_UINT && format <= VK_FORMAT_R32G32B32_SFLOAT) size = 12;
			else if (format >= VK_FORMAT_R32G32B32A32_UINT && format <= VK_FORMAT_R32G32B32A32_SFLOAT)
				size = 16;
			else if (format >= VK_FORMAT_R64_UINT && format <= VK_FORMAT_R64_SFLOAT) size = 8;
			else if (format >= VK_FORMAT_R64G64_UINT && format <= VK_FORMAT_R64G64_SFLOAT) size = 16;
			else if (format >= VK_FORMAT_R64G64B64_UINT && format <= VK_FORMAT_R64G64B64_SFLOAT) size = 24;
			else if (format >= VK_FORMAT_R64G64B64A64_UINT && format <= VK_FORMAT_R64G64B64A64_SFLOAT)
				size = 32;
			else if (format == VK_FORMAT_B10G11R11_UFLOAT_PACK32 || format == VK_FORMAT_E5B9G9R9_UFLOAT_PACK32
				|| format == VK_FORMAT_X8_D24_UNORM_PACK32 || format == VK_FORMAT_D32_SFLOAT
				|| format == VK_FORMAT_D24_UNORM_S8_UINT)
				size = 4;
			else if (format == VK_FORMAT_D16_UNORM) size = 2;
			else if (format == VK_FORMAT_D16_UNORM_S8_UINT) size = 3;
			else if (format == VK_FORMAT_D32_SFLOAT_S8_UINT) size = 5;
			else if (format == VK_FORMAT_BC1_RGB_UNORM_BLOCK || format == VK_FORMAT_BC1_RGB_SRGB_BLOCK
				|| format == VK_FORMAT_BC1_RGBA_UNORM_BLOCK || format == VK_FORMAT_BC1_RGBA_SRGB_BLOCK
				|| format == VK_FORMAT_BC4_SNORM_BLOCK || format == VK_FORMAT_BC4_UNORM_BLOCK)
				size = 8;
			else if (format == VK_FORMAT_BC2_SRGB_BLOCK || format == VK_FORMAT_BC2_UNORM_BLOCK
				|| format == VK_FORMAT_BC3_SRGB_BLOCK || format == VK_FORMAT_BC3_UNORM_BLOCK
				|| format == VK_FORMAT_BC5_SNORM_BLOCK || format == VK_FORMAT_BC5_UNORM_BLOCK
				|| format == VK_FORMAT_BC6H_SFLOAT_BLOCK || format == VK_FORMAT_BC6H_UFLOAT_BLOCK
				|| format == VK_FORMAT_BC7_SRGB_BLOCK || format == VK_FORMAT_BC7_UNORM_BLOCK)
				size = 16;
			else if (format == VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK || format == VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK
				|| format == VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK
				|| format == VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK
				|| format == VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK
				|| format == VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK || format == VK_FORMAT_EAC_R11_UNORM_BLOCK
				|| format == VK_FORMAT_EAC_R11_SNORM_BLOCK || format == VK_FORMAT_EAC_R11G11_UNORM_BLOCK
				|| format == VK_FORMAT_EAC_R11G11_SNORM_BLOCK)
				size = 16;
			else if (format == VK_FORMAT_ASTC_4x4_UNORM_BLOCK || format == VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK
				|| format == VK_FORMAT_ASTC_4x4_SRGB_BLOCK || format == VK_FORMAT_ASTC_5x4_UNORM_BLOCK
				|| format == VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK || format == VK_FORMAT_ASTC_5x4_SRGB_BLOCK
				|| format == VK_FORMAT_ASTC_5x5_UNORM_BLOCK || format == VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK
				|| format == VK_FORMAT_ASTC_5x5_SRGB_BLOCK || format == VK_FORMAT_ASTC_6x5_UNORM_BLOCK
				|| format == VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK || format == VK_FORMAT_ASTC_6x5_SRGB_BLOCK
				|| format == VK_FORMAT_ASTC_6x6_UNORM_BLOCK || format == VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK
				|| format == VK_FORMAT_ASTC_6x6_SRGB_BLOCK || format == VK_FORMAT_ASTC_8x5_UNORM_BLOCK
				|| format == VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK || format == VK_FORMAT_ASTC_8x5_SRGB_BLOCK
				|| format == VK_FORMAT_ASTC_8x6_UNORM_BLOCK || format == VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK
				|| format == VK_FORMAT_ASTC_8x6_SRGB_BLOCK || format == VK_FORMAT_ASTC_8x8_UNORM_BLOCK
				|| format == VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK || format == VK_FORMAT_ASTC_8x8_SRGB_BLOCK
				|| format == VK_FORMAT_ASTC_10x5_UNORM_BLOCK || format == VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK
				|| format == VK_FORMAT_ASTC_10x5_SRGB_BLOCK || format == VK_FORMAT_ASTC_10x6_UNORM_BLOCK
				|| format == VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK || format == VK_FORMAT_ASTC_10x6_SRGB_BLOCK
				|| format == VK_FORMAT_ASTC_10x8_UNORM_BLOCK || format == VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK
				|| format == VK_FORMAT_ASTC_10x8_SRGB_BLOCK || format == VK_FORMAT_ASTC_10x10_UNORM_BLOCK
				|| format == VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK || format == VK_FORMAT_ASTC_10x10_SRGB_BLOCK
				|| format == VK_FORMAT_ASTC_12x10_UNORM_BLOCK
				|| format == VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK || format == VK_FORMAT_ASTC_12x10_SRGB_BLOCK
				|| format == VK_FORMAT_ASTC_12x12_UNORM_BLOCK || format == VK_FORMAT_ASTC_12x12_SRGB_BLOCK
				|| format == VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK)
				size = 16;
			else if (format == VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG
				|| format == VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG
				|| format == VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG
				|| format == VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG
				|| format == VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG
				|| format == VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG
				|| format == VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG
				|| format == VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG)
				size = 8;
			else if (format == VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16
				|| format == VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16
				|| format == VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16
				|| format == VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16
				|| format == VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16
				|| format == VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16
				|| format == VK_FORMAT_G16B16G16R16_422_UNORM
				|| format == VK_FORMAT_B16G16R16G16_422_UNORM)
				size = 8;
			else if (format == VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16
				|| format == VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16
				|| format == VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16
				|| format == VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16
				|| format == VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16)
				size = 6;

			map[format] = size;
		}

		return map;
	}

	const std::unordered_map<VkFormat, u32> FORMAT_SIZE_MAP = MakeFormatSizeMap();
	VkImageType VkImageUtils::GetVkImgType(ImageType type)
	{
		switch (type)
		{
		case ImageType::Image1D:
			return VK_IMAGE_TYPE_1D;
		case ImageType::Image2D:
			return VK_IMAGE_TYPE_2D;
		case ImageType::Image3D:
			return VK_IMAGE_TYPE_3D;
		default:
			return VK_IMAGE_TYPE_2D;
		}
	}

	VkFormat VkImageUtils::GetVkImgFormat(ImageFormat format)
	{
		switch (format)
		{
		case ImageFormat::Undefined:
			return VK_FORMAT_UNDEFINED;

		case ImageFormat::R8_UNorm:
			return VK_FORMAT_R8_UNORM;
		case ImageFormat::R8_SNorm:
			return VK_FORMAT_R8_SNORM;
		case ImageFormat::R8_UInt:
			return VK_FORMAT_R8_UINT;
		case ImageFormat::R8_SInt:
			return VK_FORMAT_R8_SINT;
		case ImageFormat::R8_SRGB:
			return VK_FORMAT_R8_SRGB;

		case ImageFormat::R8G8_UNorm:
			return VK_FORMAT_R8G8_UNORM;
		case ImageFormat::R8G8_SNorm:
			return VK_FORMAT_R8G8_SNORM;
		case ImageFormat::R8G8_UInt:
			return VK_FORMAT_R8G8_UINT;
		case ImageFormat::R8G8_SInt:
			return VK_FORMAT_R8G8_SINT;
		case ImageFormat::R8G8_SRGB:
			return VK_FORMAT_R8G8_SRGB;

		case ImageFormat::R8G8B8_UNorm:
			return VK_FORMAT_R8G8B8_UNORM;
		case ImageFormat::R8G8B8_SNorm:
			return VK_FORMAT_R8G8B8_SNORM;
		case ImageFormat::R8G8B8_UInt:
			return VK_FORMAT_R8G8B8_UINT;
		case ImageFormat::R8G8B8_SInt:
			return VK_FORMAT_R8G8B8_SINT;
		case ImageFormat::R8G8B8_SRGB:
			return VK_FORMAT_R8G8B8_SRGB;

		case ImageFormat::R8G8B8A8_UNorm:
			return VK_FORMAT_R8G8B8A8_UNORM;
		case ImageFormat::R8G8B8A8_SNorm:
			return VK_FORMAT_R8G8B8A8_SNORM;
		case ImageFormat::R8G8B8A8_UInt:
			return VK_FORMAT_R8G8B8A8_UINT;
		case ImageFormat::R8G8B8A8_SInt:
			return VK_FORMAT_R8G8B8A8_SINT;
		case ImageFormat::R8G8B8A8_SRGB:
			return VK_FORMAT_R8G8B8A8_SRGB;

		case ImageFormat::R16_SFloat:
			return VK_FORMAT_R16_SFLOAT;
		case ImageFormat::R16G16_SFloat:
			return VK_FORMAT_R16G16_SFLOAT;
		case ImageFormat::R16G16B16_SFloat:
			return VK_FORMAT_R16G16B16_SFLOAT;
		case ImageFormat::R16G16B16A16_SFloat:
			return VK_FORMAT_R16G16B16A16_SFLOAT;

		case ImageFormat::R32_SFloat:
			return VK_FORMAT_R32_SFLOAT;
		case ImageFormat::R32G32_SFloat:
			return VK_FORMAT_R32G32_SFLOAT;
		case ImageFormat::R32G32B32_SFloat:
			return VK_FORMAT_R32G32B32_SFLOAT;
		case ImageFormat::R32G32B32A32_SFloat:
			return VK_FORMAT_R32G32B32A32_SFLOAT;

		case ImageFormat::D16_UNorm:
			return VK_FORMAT_D16_UNORM;
		case ImageFormat::D32_SFloat:
			return VK_FORMAT_D32_SFLOAT;
		case ImageFormat::S8_UInt:
			return VK_FORMAT_S8_UINT;
		case ImageFormat::D24_UNorm_S8_UInt:
			return VK_FORMAT_D24_UNORM_S8_UINT;
		case ImageFormat::D32_SFloat_S8_UInt:
			return VK_FORMAT_D32_SFLOAT_S8_UINT;

		default:
			return VK_FORMAT_UNDEFINED;
		}
	}

	VkImageUsageFlags VkImageUtils::GetVkImgUsage(ImageUsage usage)
	{
		VkImageUsageFlags flags = 0;

		for (auto u : magic_enum::enum_values<ImageUsage>())
		{
			if (HasFlag(usage, u))
			{
				switch (u)
				{
				case ImageUsage::TransferSrc:
					flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT; break;
				case ImageUsage::TransferDst: 
					flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT; break;
				case ImageUsage::Sampled:
					flags |= VK_IMAGE_USAGE_SAMPLED_BIT; break;
				case ImageUsage::Storage:
					flags |= VK_IMAGE_USAGE_STORAGE_BIT; break;
				case ImageUsage::ColorAttachment:
					flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; break;
				case ImageUsage::DepthStencil:
					flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT; break;
				case ImageUsage::TransientAttachment:
					flags |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT; break;
				case ImageUsage::InputAttachment:
					flags |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT; break;
				case ImageUsage::HostTransfer:
					flags |= VK_IMAGE_USAGE_HOST_TRANSFER_BIT; break;
				case ImageUsage::VideoEncodeDst:
					flags |= VK_IMAGE_USAGE_VIDEO_ENCODE_DST_BIT_KHR; break;
				case ImageUsage::VideoEncodeSrc:
					flags |= VK_IMAGE_USAGE_VIDEO_ENCODE_SRC_BIT_KHR; break;
				case ImageUsage::VideoEncodeDpb:
					flags |= VK_IMAGE_USAGE_VIDEO_ENCODE_DPB_BIT_KHR; break;
				case ImageUsage::VideoDecodeDst:
					flags |= VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR; break;
				case ImageUsage::VideoDecodeSrc:
					flags |= VK_IMAGE_USAGE_VIDEO_DECODE_SRC_BIT_KHR; break;
				case ImageUsage::VideoDecodeDpb:
					flags |= VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR; break;
				default:
					break;
				}
			}
		}

		return flags;
	}

	VkImageAspectFlags VkImageUtils::GetVkAspectMask(ImageAspect aspect)
	{
		switch (aspect)
		{
		case ImageAspect::Color:
			return VK_IMAGE_ASPECT_COLOR_BIT;
		case ImageAspect::Depth:
			return VK_IMAGE_ASPECT_DEPTH_BIT;
		case ImageAspect::Stencil:
			return VK_IMAGE_ASPECT_STENCIL_BIT;
		case ImageAspect::Meta:
			return VK_IMAGE_ASPECT_METADATA_BIT;
		default:
			return VK_IMAGE_ASPECT_COLOR_BIT;
		}
	}

	VkImageViewType VkImageUtils::GetVkViewType(ImageViewType type)
	{
		switch (type)
		{
		case ImageViewType::View1D:        
			return VK_IMAGE_VIEW_TYPE_1D;
		case ImageViewType::View2D:        
			return VK_IMAGE_VIEW_TYPE_2D;
		case ImageViewType::View3D:        
			return VK_IMAGE_VIEW_TYPE_3D;
		case ImageViewType::ViewCube:      
			return VK_IMAGE_VIEW_TYPE_CUBE;
		case ImageViewType::View1DArray:   
			return VK_IMAGE_VIEW_TYPE_1D_ARRAY;
		case ImageViewType::View2DArray:   
			return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
		case ImageViewType::View3DArray:   
			return VK_IMAGE_VIEW_TYPE_3D;
		case ImageViewType::ViewCubeArray: 
			return VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
		default:                           
			return VK_IMAGE_VIEW_TYPE_2D;
		}
	}

	VkPresentModeKHR VkImageUtils::GetVkPresentMode(PresentMode mode)
	{
		switch (mode)
		{
		case PresentMode::Fifo:
			return VK_PRESENT_MODE_FIFO_KHR;
		case PresentMode::FifoRelaxed:
			return VK_PRESENT_MODE_FIFO_RELAXED_KHR;
		case PresentMode::Immediate:
			return VK_PRESENT_MODE_IMMEDIATE_KHR;
		case PresentMode::Mailbox:
			return VK_PRESENT_MODE_MAILBOX_KHR;
		default:
			return VK_PRESENT_MODE_IMMEDIATE_KHR;
		}
	}

	u32 VkImageUtils::GetVkImgSize(ImageFormat type)
	{
		VkFormat format = VkImageUtils::GetVkImgFormat(type);
		auto size = FORMAT_SIZE_MAP.find(format);
		if (size == FORMAT_SIZE_MAP.end() || size->second == 0) [[unlikely]]
		{
			Log::Terminal(LogType::Fatal, "Render", "VkFormat not found in the map: {}", magic_enum::enum_name(format));
			exit(-1);
		}
		return size->second;
	}

}
