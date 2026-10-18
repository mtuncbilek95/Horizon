#include "ImportPipelineContext.h"

#include <Runtime/Log/Terminal.h>

namespace Horizon::Editor
{
	Guid ImportPipelineContext::ResolveAssetRef(std::string_view sourceRelativePath, std::string_view subAssetKey) const
	{
		if (!pDependencies)
			return Guid();

		for (const ImportDependency& dependency : *pDependencies)
		{
			if (dependency.kind != ImportDependencyKind::Asset)
				continue;

			if (dependency.sourceRelativePath != sourceRelativePath || dependency.subAssetKey != subAssetKey)
				continue;

			return dependency.resolvedId;
		}

		Terminal::Warn("ImportContext", "{} was never declared as a dependency, reference stays empty", sourceRelativePath);
		return Guid();
	}
}