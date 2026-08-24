#pragma once

#include <Engine/Asset/Asset.h>

#include <string>

namespace Horizon::Engine
{
	HCLASS();
	class H_EXPORT TextAsset final : public Asset
	{
		HORIZON_TYPE_REFLECT(TextAsset);
		friend class TextLoadStrategy;
	public:
		TextAsset() = default;
		~TextAsset() = default;

		const std::string& GetContent() const { return m_content; }

	private:
		std::string m_content;
	};
}