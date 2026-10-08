#include "ConsoleView.h"

#include <Editor/Renderer/Utils/ImGuiUtils.h>

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

namespace Horizon::Editor
{
	namespace
	{
		constexpr u32 LevelBit(LogLevel level)
		{
			return 1u << static_cast<u32>(level);
		}

		const c8* LevelName(LogLevel level)
		{
			switch (level)
			{
			case LogLevel::Debug:   return "Debug";
			case LogLevel::Log:     return "Log";
			case LogLevel::Info:    return "Info";
			case LogLevel::Warning: return "Warning";
			case LogLevel::Error:   return "Error";
			case LogLevel::Fatal:   return "Fatal";
			default:                return "Log";
			}
		}

		ImVec4 LevelColor(LogLevel level)
		{
			switch (level)
			{
			case LogLevel::Debug:   return ImGuiUtils::Hex("#4FA9F5");
			case LogLevel::Log:     return ImGuiUtils::Hex("#8A8A8A");
			case LogLevel::Info:    return ImGuiUtils::Hex("#3FB950");
			case LogLevel::Warning: return ImGuiUtils::Hex("#E5C07B");
			case LogLevel::Error:   return ImGuiUtils::Hex("#F14C4C");
			case LogLevel::Fatal:   return ImGuiUtils::Hex("#C678DD");
			default:                return ImGuiUtils::Hex("#8A8A8A");
			}
		}

		const ImVec4 BracketColor = ImGuiUtils::Hex("#B0B0B0");
		const ImVec4 TimestampColor = ImGuiUtils::Hex("#D0D0D0");
		const ImVec4 TitleColor = ImGuiUtils::Hex("#FF8700");
		const ImVec4 MessageColor = ImGuiUtils::Hex("#E6E6E6");

		void Piece(const ImVec4& color, const c8* pText)
		{
			ImGui::TextColored(color, "%s", pText);
			ImGui::SameLine(0.0f, 0.0f);
		}
	}

	void ConsoleView::OnInvoke()
	{
		PullHistory();
	}

	void ConsoleView::OnRender(const Engine::EngineFrame& context)
	{
		PullHistory();
		RenderToolbar();
		ImGui::Separator();

		m_visible.Clear();

		for (usize i = 0; i < m_entries.GetCount(); ++i)
		{
			if (PassesFilter(m_entries.At(i)))
				m_visible.PushBack(i);
		}

		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGuiUtils::Hex("#121212"));
		if (!ImGui::BeginChild("##ConsoleScroll", ImVec2(0.0f, 0.0f), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar))
		{
			ImGui::EndChild();
			ImGui::PopStyleColor();
			return;
		}

		if (m_dropped > 0)
			ImGui::TextDisabled("%llu older messages were dropped by the history buffer", static_cast<u64>(m_dropped));

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 2.0f));

		ImGuiListClipper clipper;
		clipper.Begin(static_cast<i32>(m_visible.GetCount()));

		while (clipper.Step())
		{
			for (i32 row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row)
				RenderEntry(m_entries.At(m_visible[static_cast<usize>(row)]));
		}

		clipper.End();
		ImGui::PopStyleVar();

		if (m_autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 1.0f)
			ImGui::SetScrollHereY(1.0f);

		ImGui::EndChild();
		ImGui::PopStyleColor();
	}

	void ConsoleView::PullHistory()
	{
		m_scratch.Clear();
		const u64 previous = m_lastSequence;
		m_lastSequence = Terminal::CopyHistory(previous, m_scratch);

		if (m_scratch.IsEmpty())
			return;

		if (previous != 0 && m_scratch.Front().sequence > previous + 1)
			m_dropped += m_scratch.Front().sequence - previous - 1;

		for (LogEntry& entry : m_scratch)
			m_entries.PushBack(std::move(entry));
	}

	void ConsoleView::RenderToolbar()
	{
		if (ImGui::Button(ICON_FA_BAN " Clear"))
		{
			m_entries.Clear();
			m_dropped = 0;
		}

		ImGui::SameLine();
		ImGui::Checkbox("Auto-scroll", &m_autoScroll);

		ImGui::SameLine();
		ImGui::SetNextItemWidth(220.0f);
		ImGui::InputTextWithHint("##ConsoleSearch", ICON_FA_MAGNIFYING_GLASS " Filter...", &m_search);

		constexpr LogLevel Levels[] = { LogLevel::Debug, LogLevel::Log, LogLevel::Info, LogLevel::Warning, LogLevel::Error, LogLevel::Fatal };

		for (LogLevel level : Levels)
		{
			ImGui::SameLine();

			const u32 bit = LevelBit(level);
			b8 enabled = (m_levelMask & bit) != 0;

			ImGui::PushStyleColor(ImGuiCol_Text, enabled ? LevelColor(level) : ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));

			if (ImGui::Checkbox(LevelName(level), &enabled))
				m_levelMask = enabled ? (m_levelMask | bit) : (m_levelMask & ~bit);

			ImGui::PopStyleColor();
		}
	}

	void ConsoleView::RenderEntry(const LogEntry& entry)
	{
		ImGui::PushID(static_cast<i32>(entry.sequence));

		Piece(BracketColor, "[");
		Piece(TimestampColor, entry.timestamp.c_str());
		Piece(BracketColor, "][");
		Piece(TitleColor, entry.title.c_str());
		Piece(BracketColor, "][");
		Piece(LevelColor(entry.level), LevelName(entry.level));
		Piece(BracketColor, "]: ");
		ImGui::TextColored(MessageColor, "%s", entry.message.c_str());

		ImGui::PopID();
	}

	b8 ConsoleView::PassesFilter(const LogEntry& entry) const
	{
		if ((m_levelMask & LevelBit(entry.level)) == 0)
			return false;

		if (m_search.empty())
			return true;

		return entry.message.find(m_search) != std::string::npos || entry.title.find(m_search) != std::string::npos;
	}
}
