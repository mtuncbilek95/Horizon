#pragma once

#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Editor
{
	class EDITOR_API ImportPipelineInfoAttribute : public Reflect::Attribute 
	{
		HORIZON_ATTRIBUTE_REFLECT(ImportPipelineInfoAttribute);
	public:
		ImportPipelineInfoAttribute(Reflect::TypeHandle handl, const List<std::string>& extensions) : m_assetType(handl), 
			m_extensions(extensions)
		{
		}
		~ImportPipelineInfoAttribute() = default;

		Reflect::TypeHandle GetWorkingAssetType() const { return m_assetType; }
		const List<std::string>& GetExtensions() const { return m_extensions; }

	private:
		Reflect::TypeHandle m_assetType;
		List<std::string> m_extensions;
	};
}