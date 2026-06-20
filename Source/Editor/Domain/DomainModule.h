#pragma once

#include <Engine/Core/Submodule.h>

#include <filesystem>

namespace Horizon
{
	class Window;

	class DomainModule final : public Submodule
	{
	public:
		DomainModule() = default;
		~DomainModule() = default;

		void OnAttach(Engine* engine) final;
		void OnSync() final;
		void OnDetach() final;

		void ImportAsset(const std::filesystem::path& from, const std::filesystem::path& to);

	private:

	};
}