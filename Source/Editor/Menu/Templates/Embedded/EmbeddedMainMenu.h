#pragma once

#include <Editor/Menu/MenuAttribute.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon
{
	HCLASS();
	class EmbeddedMainMenu
	{
		// --- Top-level ---
		HATTRIBUTE(MainMenuAttribute["File", 10u]);
		HATTRIBUTE(MainMenuAttribute["Edit", 20u]);
		HATTRIBUTE(MainMenuAttribute["Assets", 30u]);
		//HATTRIBUTE(MainMenuAttribute["View", 60u]);
		HATTRIBUTE(MainMenuAttribute["Window", 70u]);
		HATTRIBUTE(MainMenuAttribute["Tools", 80u]);
		//HATTRIBUTE(MainMenuAttribute["Build", 90u]);
		HATTRIBUTE(MainMenuAttribute["Debug", 100u]);
		//HATTRIBUTE(MainMenuAttribute["Help", 110u]);

		// --- Assets ---
		HATTRIBUTE(MainMenuAttribute["Assets/Create", 10u]);
		HATTRIBUTE(MainMenuAttribute["Assets/Import", 20u]);

		// --- View ---
		//HATTRIBUTE(MainMenuAttribute["View/Gizmos", 10u]);
		//HATTRIBUTE(MainMenuAttribute["View/Layouts", 20u]);

		// --- Window ---
		HATTRIBUTE(MainMenuAttribute["Window/Panels", 10u]);
		HATTRIBUTE(MainMenuAttribute["Window/Analysis", 20u]);
		HATTRIBUTE(MainMenuAttribute["Window/Rendering", 30u]);
		HATTRIBUTE(MainMenuAttribute["Window/Animation", 40u]);
		HATTRIBUTE(MainMenuAttribute["Window/Asset Management", 50u]);

		// --- Tools ---
		HATTRIBUTE(MainMenuAttribute["Tools/Profiling", 20u]);
		HATTRIBUTE(MainMenuAttribute["Tools/Baking", 30u]);
		HATTRIBUTE(MainMenuAttribute["Tools/Automation", 40u]);

		// --- Build ---
		//HATTRIBUTE(MainMenuAttribute["Build/Platforms", 10u]);

		// --- Debug ---
		HATTRIBUTE(MainMenuAttribute["Debug/Visualizers", 10u]);
		HATTRIBUTE(MainMenuAttribute["Debug/Logging", 20u]);

		// --- Help ---
		//HATTRIBUTE(MainMenuAttribute["Help/Documentation", 10u]);
	};
}