#pragma once

#include <Engine/Core/System.h>
#include <Runtime/PAL/Timer/Timer.h>
#include <string>
#include <string_view>

namespace Horizon::Editor
{
	class DomainFolder;
	class DomainFile;

	class H_EXPORT DomainSystem : public Engine::System
	{
		static constexpr f64 ScanIntervalSeconds = 0.5;
		static constexpr std::string_view MetaFileExt = ".hmeta";

	public:
		DomainSystem(const std::string& projectPath);
		~DomainSystem();

		Engine::AppReport OnAttach(Engine::Application* pEngine) final;
		void OnSync() final;
		void OnDetach() final;

		void GetInitializeOrder(Engine::OrderRules& rules) const final;
		void GetExecutionOrder(Engine::OrderRules& rules) const final;

		DomainFolder* GetRootFolder() const { return m_root; }

	private:
		void UpdateFolder(DomainFolder* pTarget);
		DomainFolder* FindFolder(const List<DomainFolder*>& folders, const std::string& folderName);

	private:
		std::string m_projectPath;
		DomainFolder* m_root;

		PAL::Timer m_lastSpan;
	};
}