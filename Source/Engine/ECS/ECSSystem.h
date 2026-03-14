#pragma once

#include <Engine/System/System.h>
#include <Engine/ECS/IECS.h>

namespace Horizon
{
	class ECSSystem : public System<ECSSystem>
	{
	public:
		ECSSystem() = default;
		virtual ~ECSSystem() = default;

		SystemReport OnInitialize() override;
		void OnSync() override;
		void OnFinalize() override;

	private:
		std::vector<std::shared_ptr<IECS>> m_ecsSystems;
	};
}