#include "Job.h"

namespace Horizon::Engine
{
	Job::Job(JobFunction function, void* pData) : execute(function), userData(pData)
	{
	}

	Job::Job(JobFunction function, JobFunction release, void* pData) : execute(function), discard(release), userData(pData)
	{
	}

	Job::Job(Job&& other) noexcept : execute(other.execute), discard(other.discard), userData(other.userData)
	{
		other.Release();
	}

	Job& Job::operator=(Job&& other) noexcept
	{
		if (this == &other)
			return *this;

		Discard();

		execute = other.execute;
		discard = other.discard;
		userData = other.userData;
		other.Release();

		return *this;
	}

	Job::~Job()
	{
		Discard();
	}

	void Job::Execute()
	{
		JobFunction function = execute;
		void* data = userData;

		Release();
		function(data);
	}

	void Job::Discard()
	{
		if (discard)
			discard(userData);

		Release();
	}

	void Job::Release()
	{
		execute = nullptr;
		discard = nullptr;
		userData = nullptr;
	}
}