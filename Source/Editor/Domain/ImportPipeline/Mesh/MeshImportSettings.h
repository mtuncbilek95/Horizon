#pragma once

#include <Editor/Domain/ImportPipeline/ImportSettings.h>

namespace Horizon::Editor
{
	HCLASS();
	class MeshImportSettings : public ImportSettings
	{
		HORIZON_DATA_REFLECT(MeshImportSettings);
	public:

	};
}