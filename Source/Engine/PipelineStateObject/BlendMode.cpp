#include "BlendMode.h"

namespace Horizon
{
	BlendAttachment BlendMode::blendAttachmentFromMode(BlendMode::Values mode)
	{
		switch (mode)
		{
		case BlendMode::Values::Opaque:
			return BlendAttachment()
				.setBlendEnable(false)
				.setColorMask(ColorComponent::All);

		case BlendMode::Values::AlphaBlend:
			return BlendAttachment()
				.setBlendEnable(true)
				.setSrcColor(BlendFactor::SrcAlpha)
				.setDstColor(BlendFactor::OneMinusSrcAlpha)
				.setColorOp(BlendOp::Add)
				.setSrcAlpha(BlendFactor::One)
				.setDstAlpha(BlendFactor::OneMinusSrcAlpha)
				.setAlphaOp(BlendOp::Add)
				.setColorMask(ColorComponent::All);

		case BlendMode::Values::Additive:
			return BlendAttachment()
				.setBlendEnable(true)
				.setSrcColor(BlendFactor::One)
				.setDstColor(BlendFactor::One)
				.setColorOp(BlendOp::Add)
				.setSrcAlpha(BlendFactor::One)
				.setDstAlpha(BlendFactor::One)
				.setAlphaOp(BlendOp::Add)
				.setColorMask(ColorComponent::All);

		case BlendMode::Values::Premultiplied:
			return BlendAttachment()
				.setBlendEnable(true)
				.setSrcColor(BlendFactor::One)
				.setDstColor(BlendFactor::OneMinusSrcAlpha)
				.setColorOp(BlendOp::Add)
				.setSrcAlpha(BlendFactor::One)
				.setDstAlpha(BlendFactor::OneMinusSrcAlpha)
				.setAlphaOp(BlendOp::Add)
				.setColorMask(ColorComponent::All);
		}
	}
}