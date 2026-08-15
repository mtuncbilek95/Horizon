#pragma once

#include <Editor/Domain/ImportSettings/ImportSettings.h>
#include <Runtime/Containers/Guid.h>
#include <Runtime/PAL/Timer/DateTime.h>
#include <Runtime/RTTR/Reflection.h>
#include <string>

namespace Horizon::Editor
{
	HCLASS();
	struct H_EXPORT DomainMetaDescriptor : public Reflect::Base
	{
		HORIZON_TYPE_REFLECT(DomainMetaDescriptor);

		HFIELD();
		Guid id;

		HFIELD();
		std::string assetTypeName;

		HFIELD();
		ImportSettings* pSettings;
	};
}