#pragma once

#include <Editor/Domain/ImportPipeline/ImportSettings.h>

namespace Horizon::TestLibrary
{
	HCLASS();
	class TestImportSettings : public Editor::ImportSettings
	{
		HORIZON_DATA_REFLECT(TestImportSettings);
	public:

	};
}