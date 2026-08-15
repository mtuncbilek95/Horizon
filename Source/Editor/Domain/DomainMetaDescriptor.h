#pragma once

#include <Runtime/Containers/Guid.h>
#include <Runtime/PAL/Timer/DateTime.h>
#include <Runtime/RTTR/Reflection.h>
#include <string>

namespace Horizon
{
	HCLASS();
	struct H_EXPORT DomainMetaDescriptor
	{
		HFIELD();
		Guid id;

		HFIELD();
		std::string sourcePath;

		HFIELD();
		PAL::DateTime lastModifiedTime;
	};
}