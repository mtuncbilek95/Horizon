#include "WorldCommandBuffer.h"

namespace Horizon::Engine
{
	EntityHandle WorldCommandBuffer::CreateEntity(World& world)
	{
		EntityHandle entity = world.GetEntities().Reserve();
		if (!entity.IsValid())
			return entity;

		Record([entity](World& target)
			{
				target.GetEntities().Commit(entity);
			});

		return entity;
	}

	void WorldCommandBuffer::DestroyEntity(EntityHandle entity)
	{
		Record([entity](World& target)
			{
				target.DestroyEntity(entity);
			});
	}

	void WorldCommandBuffer::Record(std::function<void(World&)>&& action)
	{
		Command command;
		command.sequence = s_sequence.FetchAdd(1);
		command.action = std::move(action);

		m_commands.PushBack(std::move(command));
	}
}