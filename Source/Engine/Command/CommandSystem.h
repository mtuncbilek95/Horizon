#pragma once

#include <Engine/Core/System.h>

#include <filesystem>

namespace Horizon
{
	class H_EXPORT CommandSystem : public System
	{
	public:
		CommandSystem(i32 argC, c8** argV);
		~CommandSystem() = default;

		SystemReport OnAttach(Engine* pEngine);
		void OnDetach();

		void GetInitializeOrder(OrderRules& rules) const final;

		const std::filesystem::path& GetExecutionPath() const { return m_executionPath; }
		const std::filesystem::path& GetProjectPath() const { return m_projectPath; }

	private:
		std::filesystem::path m_executionPath;
		std::filesystem::path m_projectPath;
	};
}