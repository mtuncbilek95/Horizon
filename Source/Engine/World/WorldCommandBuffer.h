#pragma once

#include <Engine/World/World.h>

#include <Runtime/Containers/List.h>
#include <Runtime/PAL/Sync/Atomic.h>

#include <functional>

namespace Horizon::Engine
{
	class WorldCommandBuffer final
	{
		friend class WorldService;
	public:
		WorldCommandBuffer() = default;
		~WorldCommandBuffer() = default;

		EntityHandle CreateEntity(World& world);

		void DestroyEntity(EntityHandle entity);

		template<typename T>
		void AddComponent(EntityHandle entity, T&& component)
		{
			Record([entity, payload = std::move(component)](World& target) mutable
				{
					target.AddComponent<T>(entity, std::move(payload));
				});
		}

		template<typename T>
		void RemoveComponent(EntityHandle entity)
		{
			Record([entity](World& target)
				{
					target.RemoveComponent<T>(entity);
				});
		}

		usize GetCount() const { return m_commands.GetCount(); }
		void Clear() { m_commands.Clear(); }

	private:
		struct Command
		{
			u64 sequence = 0;
			std::function<void(World&)> action;
		};

		void Record(std::function<void(World&)>&& action);

	private:
		inline static PAL::Atomic<u64> s_sequence = 0;
		List<Command> m_commands;
	};
}