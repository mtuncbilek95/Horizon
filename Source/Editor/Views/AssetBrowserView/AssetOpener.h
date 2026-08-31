#pragma once

#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	class Engine;
}

namespace Horizon::Editor
{
	class DomainFile;

	HCLASS();
	class H_EXPORT AssetOpener : public Reflect::Base
	{
	public:
		virtual ~AssetOpener() = default;

		virtual b8 Open(Engine::Engine* pEngine, DomainFile* pFile) = 0;
	};
}