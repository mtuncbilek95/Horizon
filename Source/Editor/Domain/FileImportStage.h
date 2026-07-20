#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon
{
	class ImportContext;

	class FileImportStage
	{
	public:
		virtual ~FileImportStage() = default;

		virtual b8 ShouldRun(const ImportContext& context) const { return true; }
		virtual void OnRun(ImportContext& context) = 0;
	};
}