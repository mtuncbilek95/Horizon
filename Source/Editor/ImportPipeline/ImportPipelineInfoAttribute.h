#pragma once

#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Editor
{
	class EDITOR_API ImportPipelineInfoAttribute : public Reflect::Attribute 
	{
		HORIZON_ATTRIBUTE_REFLECT(ImportPipelineInfoAttribute);
	public:
		ImportPipelineInfoAttribute(Reflect::TypeHandle assetHandl, Reflect::TypeHandle settingsHandl, const List<std::string>& extensions) : m_assetType(assetHandl),
			m_settingsHandl(settingsHandl), m_extensions(extensions)
		{
		}
		~ImportPipelineInfoAttribute() = default;

		Reflect::TypeHandle GetWorkingAssetType() const { return m_assetType; }
		Reflect::TypeHandle GetSettingsType() const { return m_settingsHandl; }
		const List<std::string>& GetExtensions() const { return m_extensions; }

	private:
		Reflect::TypeHandle m_assetType;
		Reflect::TypeHandle m_settingsHandl;
		List<std::string> m_extensions;
	};
}