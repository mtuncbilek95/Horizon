#pragma once

#include <Engine/Core/Service.h>
#include <Engine/World/World.h>
#include <Engine/World/System.h>

#include <Runtime/Containers/Guid.h>
#include <Runtime/Containers/List.h>
#include <Runtime/PAL/Sync/Mutex.h>

namespace Horizon::Engine
{
	class H_EXPORT WorldService : public Service
	{
	public:
		WorldService() = default;
		~WorldService() = default;

		ModuleReport OnInitialize() final;
		void OnExecute() final;
		void OnFinalize() final;
		void DeclareDependencies(ModuleGraph& graph) final;

		World* GetActiveWorld() const { return m_activeWorld; }
		const Guid& GetActiveSceneId() const { return m_activeSceneId; }

		b8 LoadScene(const Guid& sceneId);

		template<typename T>
			requires std::is_base_of_v<System, T>
		T* FindSystem()
		{
			for (System* pSystem : m_systems)
			{
				if (pSystem->GetTypeId() == Reflect::TypeOf<T>())
					return static_cast<T*>(pSystem);
			}

			Terminal::Error(StringOps::GetName(this), "{} is not an active system", typeid(T).name());
			return nullptr;
		}

	private:
		void FlushCommandBuffers();

	private:
		World* m_activeWorld = nullptr;
		Guid m_activeSceneId;

		// TODO: Just to test things!
		List<System*> m_systems;
	};
}