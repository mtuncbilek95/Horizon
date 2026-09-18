#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/RHI/Texture/GfxTextureFormat.h>
#include <Runtime/RHI/Pipeline/GfxVertexBinding.h>

namespace Horizon::RHI
{
	struct GfxVertexLayout
	{
		static constexpr u32 MaxAttributes = 16;
		static constexpr u32 MaxBindings = 8;

		GfxVertexAttribute attributes[MaxAttributes] = {};
		u32 attributeCount = 0;

		GfxVertexBinding bindings[MaxBindings] = {};
		u32 bindingCount = 0;

		GfxVertexLayout& AddBinding(u32 binding, u32 stride, GfxVertexInputRate rate = GfxVertexInputRate::PerVertex)
		{
			if (bindingCount >= MaxBindings)
				return *this;

			bindings[bindingCount++] = { binding, stride, rate };
			return *this;
		}

		GfxVertexLayout& AddAttribute(const c8* semantic, u32 semanticIndex, GfxTextureFormat format, u32 binding, u32 offset)
		{
			if (attributeCount >= MaxAttributes)
				return *this;

			attributes[attributeCount++] = { semantic, semanticIndex, format, binding, offset };
			return *this;
		}

		b8 IsEmpty() const { return attributeCount == 0; }
	};
}