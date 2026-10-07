#pragma once

#include <Engine/Job/JobSystem.h>

#include <Jolt/Jolt.h>
#include <Jolt/Core/JobSystemWithBarrier.h>

namespace Horizon::Engine
{
	class ENGINE_API JoltThreadBridge : public JPH::JobSystemWithBarrier
	{
		using JoltJob = JPH::JobSystem::Job;
		using JoltFunction = JPH::JobSystem::JobFunction;
		using JoltHandle = JPH::JobSystem::JobHandle;

		static void RunJolt(void* pUserData);
	public:
		JoltThreadBridge(Horizon::Engine::JobSystem* pOwner, u32 maxBarriers);
		~JoltThreadBridge() final = default;

		i32 GetMaxConcurrency() const final;
		JoltHandle CreateJob(const c8* pName, JPH::ColorArg color, const JoltFunction& function, u32 dependencyCount) final;

	protected:
		void QueueJob(JoltJob* pJob) final;
		void QueueJobs(JoltJob** ppJobs, JPH::uint count) final;
		void FreeJob(JoltJob* pJob) final;

	private:
		Horizon::Engine::JobSystem* m_owner;
	};
}