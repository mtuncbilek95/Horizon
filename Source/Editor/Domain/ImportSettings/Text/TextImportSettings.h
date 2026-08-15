#pragma once

#include <Editor/Attributes/ImportTypeAttribute.h>
#include <Editor/Domain/ImportSettings/ImportSettings.h>
#include <Engine/Asset/Text/TextAsset.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Editor
{
	HCLASS(ImportType[Reflect::TypeOf<Engine::TextAsset>(), { ".txt" }]);
	class H_EXPORT TextImportSettings : public ImportSettings
	{
		HORIZON_TYPE_REFLECT(TextImportSettings);
	public:

	private:
		HFIELD();
		b8 m_utf8 = false;
	};
}