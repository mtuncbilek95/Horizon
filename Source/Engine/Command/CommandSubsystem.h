#pragma once

#include <Engine/Core/Subsystem.h>

#include <filesystem>

namespace Horizon
{
	class H_EXPORT CommandSubsystem : public Subsystem
	{
	public:
		CommandSubsystem(i32 argC, c8** argV);
		~CommandSubsystem() = default;

		EngineReport OnAttach(Engine* pEngine);
		void OnDetach();

		void GetInitializeOrder(OrderRules& rules) const final;

	private:
		std::filesystem::path m_executionPath;
		std::filesystem::path m_projectPath;
	};
}