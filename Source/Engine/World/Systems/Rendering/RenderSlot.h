#pragma once

#include <Runtime/Containers/Memory/UniquePtr.h>
#include <Runtime/Containers/Vector/List.h>
#include <Runtime/Math/Vec2u.h>
#include <Runtime/RHI/Command/CommandList.h>
#include <Runtime/RHI/Command/CommandPool.h>
#include <Runtime/RHI/Texture/Texture.h>
#include <Runtime/RHI/Texture/TextureView.h>

namespace Horizon::Engine
{
	struct RenderSlot
	{
		UniquePtr<RHI::Texture> pTargetTexture;
		UniquePtr<RHI::TextureView> pTargetView;
		UniquePtr<RHI::Texture> pTargetDepth;
		UniquePtr<RHI::TextureView> pDepthView;
		UniquePtr<RHI::CommandPool> pTargetPool;
		UniquePtr<RHI::CommandList> pTargetCmd;

		u32 shaderView = kInvalid32;

		RHI::ResourceState currState = RHI::ResourceState::Undefined;
		Math::Vec2u currSize = { 0, 0 };

		u64 fenceValue = 0;
	};
}
