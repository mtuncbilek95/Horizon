#pragma once

#include <Runtime/Graphics/RHI/Util/ShaderStage.h>

namespace Horizon
{
	class GfxShader;

	class GfxMaterial
	{
	public:
		GfxMaterial() = default;
		virtual ~GfxMaterial() = default;

		GfxShader* GetShader(ShaderStage stage) const
		{
			auto it = m_shaderMap.find(stage);
			if (it != m_shaderMap.end())
				return it->second.get();
			return nullptr;
		}

	private:
		std::unordered_map<ShaderStage, std::shared_ptr<GfxShader>> m_shaderMap;

	};
}