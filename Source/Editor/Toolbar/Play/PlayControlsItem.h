#pragma once

#include <Editor/Toolbar/ToolBarItemAttribute.h>
#include <Editor/ToolBar/ToolBarItem.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Editor
{
	HCLASS(ToolBarItem[ToolBarSection::Center, 0]);
	class H_EXPORT PlayControlsItem final : public ToolBarItem
	{
		HORIZON_TYPE_REFLECT(PlayControlsItem);
	public:
		void OnRender() final;

	private:
		enum class State
		{
			Play,
			Pause,
			Stop
		};

		State m_state = State::Stop;
	};
}