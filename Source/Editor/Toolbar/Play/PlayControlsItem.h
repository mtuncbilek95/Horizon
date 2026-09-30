#pragma once

#include <Editor/Toolbar/ToolBarItemAttribute.h>
#include <Editor/ToolBar/ToolBarItem.h>
#include <Runtime/RTTR/Reflection.h>
#include <Runtime/Math/Color4f.h>

namespace Horizon::Editor
{
	HCLASS(ToolBarItem[ToolBarSection::Center, 0]);
	class H_EXPORT PlayControlsItem final : public ToolBarItem
	{
		HORIZON_TYPE_REFLECT(PlayControlsItem);
	public:
		void OnRender() final;

	private:
		b8 StateButton(const c8* pIcon, b8 active, const Math::Color4f& activeCol);

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