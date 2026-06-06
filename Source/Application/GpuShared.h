#pragma once

#include <glm/glm.hpp>

namespace Horizon
{
	inline constexpr u32 ShadowMapSize = 2048;

	struct FrameConstants
	{
		glm::mat4 viewProj;
		glm::mat4 invViewProj;
		glm::mat4 lightViewProj;
		glm::vec4 lightDirection;
		glm::vec4 lightColor;
		glm::vec3 camPos;
		f32 _pad;
	};

	struct GBufferPush
	{
		glm::mat4 model;
		u32 vertexBufferIndex;
		u32 materialBufferIndex;
		u32 materialIndex;
		u32 frameBufferIndex;
		u32 frameSlot;
	};

	struct ShadowPush
	{
		glm::mat4 model;
		u32 vertexBufferIndex;
		u32 frameBufferIndex;
		u32 frameSlot;
	};

	struct LightPush
	{
		u32 albedoIndex;
		u32 normalIndex;
		u32 materialIndex;
		u32 emissiveIndex;
		u32 depthIndex;
		u32 shadowIndex;
		u32 frameBufferIndex;
		u32 frameSlot;
	};

	struct SkyPush
	{
		u32 depthIndex;
		u32 frameBufferIndex;
		u32 frameSlot;
	};

	struct BloomExtractPush
	{
		u32 hdrIndex;
		f32 threshold;
	};

	struct BloomBlurPush
	{
		u32 srcIndex;
		u32 horizontal;
	};

	struct FinalPush
	{
		u32 hdrIndex;
		u32 bloomIndex;
		u32 frameBufferIndex;
		u32 frameSlot;
	};
}