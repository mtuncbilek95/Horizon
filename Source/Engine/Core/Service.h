#pragma once

#include <Engine/Core/Module.h>

namespace Horizon::Engine
{
	/*
	* Service is the main independent coordinator for
	* engine's inner system. They should not be mistaken
	* with System inside of WorldService. 
	* 
	* The examples are EditorService which controls the 
	* editor rendering and input mechanism and process them
	* with the help of WindowService and GraphicsContext
	*/
	class H_EXPORT Service : public Module
	{
	public:
		Service() = default;
		virtual ~Service() = default;

		virtual void OnExecute() = 0;
	};
}