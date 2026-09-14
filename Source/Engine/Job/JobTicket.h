#pragma once

#include <Engine/Job/Job.h>
#include <Engine/Job/JobLane.h>

namespace Horizon::Engine
{
	using SubmitTicket = u64;
	static constexpr SubmitTicket InvalidSubmitTicket = kInvalid64;

	enum class CompletionState : u8
	{
		Invalid = 0,
		Pending,
		Running,
		Completed
	};

	struct H_EXPORT SubmitRequest
	{
		JobLane lane = JobLane::Critical;
		Job job;
	};
}