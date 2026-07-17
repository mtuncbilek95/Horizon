#pragma once

#include <string>
#include <functional>

namespace Horizon
{
	class Archive;

	struct FieldManifest
	{
		std::string name;
		std::function<void(void*, Archive&)> serialize;
	};
}