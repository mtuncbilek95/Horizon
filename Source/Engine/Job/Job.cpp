#include "Job.h"

namespace Horizon::Engine
{
	Job::Job(JobFunction function, void* pData) : m_execute(function), m_userData(pData)
	{
	}

	Job::Job(JobFunction function, JobFunction release, void* pData) : m_execute(function), m_discard(release), m_userData(pData)
	{
	}

	Job::Job(Job&& other) noexcept : m_execute(other.m_execute), m_discard(other.m_discard), m_userData(other.m_userData)
	{
		other.Release();
	}

	Job& Job::operator=(Job&& other) noexcept
	{
		if (this == &other)
			return *this;

		Discard();

		m_execute = other.m_execute;
		m_discard = other.m_discard;
		m_userData = other.m_userData;
		other.Release();

		return *this;
	}

	Job::~Job()
	{
		Discard();
	}

	void Job::Execute()
	{
		JobFunction function = m_execute;
		void* pData = m_userData;

		Release();
		function(pData);
	}

	void Job::Discard()
	{
		if (m_discard)
			m_discard(m_userData);

		Release();
	}

	void Job::Release()
	{
		m_execute = nullptr;
		m_discard = nullptr;
		m_userData = nullptr;
	}
}