#include "JoltThreadBridge.h"

namespace Horizon::Engine
{
	void JoltThreadBridge::RunJolt(void* pUserData)
	{
		JoltJob* pJob = (JoltJob*)pUserData;

		pJob->Execute();
		pJob->Release();
	}

	JoltThreadBridge::JoltThreadBridge(Horizon::Engine::JobSystem* pOwner, u32 maxBarriers) : JPH::JobSystemWithBarrier(maxBarriers), m_owner(pOwner)
	{
	}

	i32 JoltThreadBridge::GetMaxConcurrency() const
	{
		return (i32)m_owner->GetWorkerCount(JobLane::Critical) + 1;
	}

	JoltThreadBridge::JoltHandle JoltThreadBridge::CreateJob(const c8* pName, JPH::ColorArg color, const JoltFunction& function, u32 dependencyCount)
	{
		JoltJob* pJob = Memory::Allocator::Create<JoltJob>(Memory::CurrLoc(), pName, color, this, function, dependencyCount);
		JoltHandle handle(pJob);

		if (dependencyCount == 0)
			QueueJob(pJob);

		return handle;
	}

	void JoltThreadBridge::QueueJob(JoltJob* pJob)
	{
		pJob->AddRef();
		m_owner->SubmitDetached(JobLane::Critical, Horizon::Engine::Job(&JoltThreadBridge::RunJolt, pJob));
	}

	void JoltThreadBridge::QueueJobs(JoltJob** ppJobs, JPH::uint count)
	{
		for (JPH::uint i = 0; i < count; i++)
			QueueJob(ppJobs[i]);
	}

	void JoltThreadBridge::FreeJob(JoltJob* pJob)
	{
		Memory::Allocator::Delete(pJob);
	}
}