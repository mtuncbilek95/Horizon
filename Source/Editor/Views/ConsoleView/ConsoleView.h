#pragma once

#include <Editor/Views/EditorViewAttribute.h>
#include <Editor/Views/ViewObject.h>
#include <Editor/Font/IconsFontAwesome6.h>
#include <Runtime/Containers/List.h>
#include <Runtime/Containers/RingBuffer.h>
#include <Runtime/Log/Terminal.h>

#include <string>

namespace Horizon::Editor
{
	HCLASS(EditorView[ICON_FA_TERMINAL, "Console", DockZone::Bottom, EditorViewFlags::OpenOnStart]);
	class EDITOR_API ConsoleView : public ViewObject
	{
		HORIZON_TYPE_REFLECT(ConsoleView);

		static constexpr usize ViewCapacity = 8192;

	public:
		ConsoleView() = default;
		~ConsoleView() = default;

		void OnInvoke() final;
		void OnRender(const Engine::EngineFrame& context) final;

		void OnLibraryRegistered(const Engine::ReflectionLibrary& library) final;
		void OnLibraryUnregistered(const Engine::ReflectionLibrary& library) final;

	private:
		void PullHistory();
		void RenderToolbar();
		void RenderEntry(const LogEntry& entry);
		b8 PassesFilter(const LogEntry& entry) const;

	private:
		RingBuffer<LogEntry> m_entries{ ViewCapacity };
		List<usize> m_visible;
		List<LogEntry> m_scratch;

		std::string m_search;
		u64 m_lastSequence = 0;
		u64 m_dropped = 0;
		u32 m_levelMask = 0xFF;
		b8 m_autoScroll = true;
	};
}
