#pragma once

#include <Editor/Import/ImportRequest.h>
#include <Editor/Import/ImportSink.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Editor
{
	HCLASS();
	class H_EXPORT AssetImporter : public Reflect::Base
	{
	public:
		virtual ~AssetImporter() = default;

		virtual b8 Import(const ImportRequest& request, ImportSink& sink) = 0;
	};
}