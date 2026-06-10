#include "CommandLineModule.h"

namespace Horizon
{
	CommandLineModule::CommandLineModule(i32 argC, c8** argV)
	{
		m_arguments.resize(argC);

		for (usize i = 0; i < m_arguments.size(); i++)
			m_arguments[i] = argV[i];
	}

	CommandLineModule::~CommandLineModule()
	{
	}
}