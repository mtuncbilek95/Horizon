#pragma once

#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Editor
{
	class EDITOR_API ImportPipelineInfoAttribute : public Reflect::Attribute 
	{
		HORIZON_ATTRIBUTE_REFLECT(ImportPipelineInfoAttribute);
	public:
		ImportPipelineInfoAttribute(const List<std::string>& extensions) : m_extensions(extensions)
		{
		}
		~ImportPipelineInfoAttribute() = default;

		const List<std::string>& GetExtensions() const { return m_extensions; }

	private:
		List<std::string> m_extensions;
	};
}