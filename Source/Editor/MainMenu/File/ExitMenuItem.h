#pragma once

#include <Editor/Attributes/MenuItemAttribute.h>
#include <Editor/MainMenu/MenuItem.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Editor
{
	HCLASS(MenuItem["File/Exit", 100]);
	class H_EXPORT ExitMenuItem : public MenuItem 
	{
	public:
		void OnExecute() final;
	};
}