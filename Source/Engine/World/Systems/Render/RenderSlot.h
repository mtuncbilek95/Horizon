#pragma once

#include <Runtime/Math/Vec2u.h>
#include <Runtime/RHI/Texture/GfxTexture.h>
#include <Runtime/RHI/Command/GfxCommandList.h>

namespace Horizon::Engine
{
	struct RenderSlot
	{
		RHI::GfxTexture* pTargetTexture = nullptr;
		RHI::GfxCommandList* pTargetCmd = nullptr;

		u32 renderTargetView = kInvalid32;
		u32 shaderView = kInvalid32;

		RHI::GfxResourceState currState = RHI::GfxResourceState::Common;
		Math::Vec2u currSize = { 0, 0 };

		u32 fenceValue = 0;
	};
}
