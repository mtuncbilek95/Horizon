#pragma once

#include <Editor/Domain/MetaHeader.h>

namespace Horizon
{
	class H_EXPORT ImportFileSettings
	{
	public:
		virtual ~ImportFileSettings() = default;

		virtual void OnImportFrom() = 0;
		virtual void OnImportDefault(MetaHeader& outHeaderToSerialize) = 0;
	};
}