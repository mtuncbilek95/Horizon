#pragma once

#include <Engine/Engine/IModule.h>

#include <string>
#include <vector>

namespace Horizon
{
	/* 
	 * Checks if engine wanna start with a command line 
	 * arguments and pass the arguments to different modules.
	 */
	class CommandLineModule : public IModule
	{
	public:
		CommandLineModule(i32 argC, c8** argV);
		~CommandLineModule();

		const std::vector<std::string>& GetArguments() { return m_arguments; }

	private:
		std::vector<std::string> m_arguments;
	};
}