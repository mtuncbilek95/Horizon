#pragma once

#include <Engine/Reflection/ReflectionContext.h>

#include <Runtime/Containers/Path.h>
#include <Runtime/PAL/Module/SymbolLibrary.h>

namespace Horizon
{
	class ReflectionModule final
	{
	public:
		void LoadMainModule();
		void LoadModule(const Path& path);

		const ReflectionContext& GetContext() const noexcept { return m_context; }

	private:
		void ProcessManifest(const SymbolLibrary& library);

	private:
		ReflectionContext m_context;
	};
}