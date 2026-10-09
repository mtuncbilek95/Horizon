#include "MaterialGraphView.h"

#include <imgui.h>
#include <imgui_node_editor.h>
#include <span>

namespace Horizon::Editor
{
	namespace
	{
		ax::NodeEditor::EditorContext* m_graphContext = nullptr;
		b8 m_firstFrame = true;
		f32 m_scalarValue = 0.5f;
		namespace ed = ax::NodeEditor;

		struct PinDesc
		{
			u64 id;
			const c8* pLabel;
			ImU32 color;
		};

		struct VerticalNodeStyle
		{
			ImU32 background;
			ImU32 border;
			ImU32 text;
			ImU32 pin;
			f32 pinWidth;
		};

		constexpr ImU32 FloatPinColor = IM_COL32(147, 226, 74, 255);
		constexpr ImU32 ColorPinColor = IM_COL32(220, 48, 48, 255);
		constexpr ImU32 VectorPinColor = IM_COL32(68, 201, 156, 255);

		constexpr f32 VerticalNodeWidth = 140.0f;
		constexpr f32 VerticalNodeRounding = 4.0f;
		constexpr f32 VerticalPinHeight = 10.0f;

		constexpr VerticalNodeStyle TreeStyle = { IM_COL32(128, 128, 128, 200), IM_COL32(32, 32, 32, 200), IM_COL32(255, 255, 255, 255), IM_COL32(60, 180, 255, 150), VerticalNodeWidth };
		constexpr VerticalNodeStyle HoudiniStyle = { IM_COL32(229, 229, 229, 200), IM_COL32(125, 125, 125, 200), IM_COL32(0, 0, 0, 255), IM_COL32(125, 125, 125, 255), 40.0f };

		void PlaceNodes()
		{
			ed::SetNodePosition(ed::NodeId(1), ImVec2(0.0f, 0.0f));
			ed::SetNodePosition(ed::NodeId(3), ImVec2(0.0f, 170.0f));
			ed::SetNodePosition(ed::NodeId(2), ImVec2(280.0f, 60.0f));
			ed::SetNodePosition(ed::NodeId(4), ImVec2(460.0f, 40.0f));
			ed::SetNodePosition(ed::NodeId(9), ImVec2(0.0f, 300.0f));
			ed::SetNodePosition(ed::NodeId(5), ImVec2(40.0f, 340.0f));
			ed::SetNodePosition(ed::NodeId(6), ImVec2(40.0f, 440.0f));
			ed::SetNodePosition(ed::NodeId(7), ImVec2(360.0f, 320.0f));
			ed::SetNodePosition(ed::NodeId(8), ImVec2(360.0f, 420.0f));
		}

		void DrawPinIcon(ImU32 color)
		{
			const f32 size = ImGui::GetTextLineHeight();
			ImGui::Dummy(ImVec2(size, size));

			const ImVec2 min = ImGui::GetItemRectMin();
			const ImVec2 max = ImGui::GetItemRectMax();
			const ImVec2 center = ImVec2((min.x + max.x) * 0.5f, (min.y + max.y) * 0.5f);

			ImGui::GetWindowDrawList()->AddCircleFilled(center, size * 0.3f, color);
			ed::PinPivotRect(center, center);
		}

		void DrawPinRow(const PinDesc& pin, ed::PinKind kind)
		{
			ed::BeginPin(ed::PinId(pin.id), kind);

			if (kind == ed::PinKind::Input)
			{
				DrawPinIcon(pin.color);
				ImGui::SameLine();
				ImGui::TextUnformatted(pin.pLabel);
			}
			else
			{
				ImGui::TextUnformatted(pin.pLabel);
				ImGui::SameLine();
				DrawPinIcon(pin.color);
			}

			ed::EndPin();
		}

		void DrawPinIcons(std::span<const PinDesc> pins, ed::PinKind kind)
		{
			ImGui::BeginGroup();

			for (const PinDesc& pin : pins)
			{
				ed::BeginPin(ed::PinId(pin.id), kind);
				DrawPinIcon(pin.color);
				ed::EndPin();
			}

			ImGui::EndGroup();
		}

		void DrawBlueprintNode(u64 nodeId, const c8* pTitle, ImU32 headerColor, std::span<const PinDesc> inputs, std::span<const PinDesc> outputs)
		{
			ed::BeginNode(ed::NodeId(nodeId));

			ImGui::TextUnformatted(pTitle);
			const f32 headerBottom = ImGui::GetItemRectMax().y + ImGui::GetStyle().ItemSpacing.y * 0.5f;
			ImGui::Dummy(ImVec2(0.0f, ImGui::GetStyle().ItemSpacing.y));

			ImGui::BeginGroup();
			for (const PinDesc& pin : inputs)
				DrawPinRow(pin, ed::PinKind::Input);
			ImGui::EndGroup();

			ImGui::SameLine(0.0f, 24.0f);

			ImGui::BeginGroup();
			for (const PinDesc& pin : outputs)
				DrawPinRow(pin, ed::PinKind::Output);
			ImGui::EndGroup();

			ed::EndNode();

			const ImVec2 position = ed::GetNodePosition(ed::NodeId(nodeId));
			const ImVec2 size = ed::GetNodeSize(ed::NodeId(nodeId));
			const ed::Style& style = ed::GetStyle();

			ImDrawList* pDrawList = ed::GetNodeBackgroundDrawList(ed::NodeId(nodeId));
			pDrawList->AddRectFilled(position, ImVec2(position.x + size.x, headerBottom), headerColor, style.NodeRounding, ImDrawFlags_RoundCornersTop);
		}

		void DrawSimpleNode(u64 nodeId, const c8* pSymbol, std::span<const PinDesc> inputs, std::span<const PinDesc> outputs)
		{
			ed::BeginNode(ed::NodeId(nodeId));

			DrawPinIcons(inputs, ed::PinKind::Input);
			ImGui::SameLine();
			ImGui::TextUnformatted(pSymbol);
			ImGui::SameLine();
			DrawPinIcons(outputs, ed::PinKind::Output);

			ed::EndNode();
		}

		void DrawVerticalPin(u64 pinId, ed::PinKind kind, const VerticalNodeStyle& style)
		{
			ImGui::Dummy(ImVec2(VerticalNodeWidth, VerticalPinHeight));

			if (pinId == 0)
				return;

			const ImVec2 itemMin = ImGui::GetItemRectMin();
			const ImVec2 itemMax = ImGui::GetItemRectMax();
			const f32 left = itemMin.x + (VerticalNodeWidth - style.pinWidth) * 0.5f;
			const ImVec2 min = ImVec2(left, itemMin.y);
			const ImVec2 max = ImVec2(left + style.pinWidth, itemMax.y);
			const ImVec2 center = ImVec2((min.x + max.x) * 0.5f, (min.y + max.y) * 0.5f);

			ImGui::GetWindowDrawList()->AddRectFilled(min, max, style.pin, VerticalNodeRounding, ImDrawFlags_RoundCornersAll);

			ed::BeginPin(ed::PinId(pinId), kind);
			ed::PinPivotRect(center, center);
			ed::PinRect(min, max);
			ed::EndPin();
		}

		void DrawVerticalNode(u64 nodeId, const c8* pTitle, u64 inputPinId, u64 outputPinId, const VerticalNodeStyle& style)
		{
			ed::PushStyleColor(ed::StyleColor_NodeBg, ImColor(style.background));
			ed::PushStyleColor(ed::StyleColor_NodeBorder, ImColor(style.border));
			ed::PushStyleVar(ed::StyleVar_NodePadding, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
			ed::PushStyleVar(ed::StyleVar_NodeRounding, VerticalNodeRounding);
			ed::PushStyleVar(ed::StyleVar_SourceDirection, ImVec2(0.0f, 1.0f));
			ed::PushStyleVar(ed::StyleVar_TargetDirection, ImVec2(0.0f, -1.0f));
			ed::PushStyleVar(ed::StyleVar_LinkStrength, 0.0f);
			ImGui::PushStyleColor(ImGuiCol_Text, style.text);

			ed::BeginNode(ed::NodeId(nodeId));

			DrawVerticalPin(inputPinId, ed::PinKind::Input, style);

			const f32 textWidth = ImGui::CalcTextSize(pTitle).x;
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (VerticalNodeWidth - textWidth) * 0.5f);
			ImGui::TextUnformatted(pTitle);

			DrawVerticalPin(outputPinId, ed::PinKind::Output, style);

			ed::EndNode();

			ImGui::PopStyleColor();
			ed::PopStyleVar(5);
			ed::PopStyleColor(2);
		}

		void DrawCommentNode(u64 nodeId, const c8* pTitle, const ImVec2& size)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.75f);
			ed::PushStyleColor(ed::StyleColor_NodeBg, ImColor(255, 255, 255, 64));
			ed::PushStyleColor(ed::StyleColor_NodeBorder, ImColor(255, 255, 255, 64));

			ed::BeginNode(ed::NodeId(nodeId));
			ImGui::TextUnformatted(pTitle);
			ed::Group(size);
			ed::EndNode();

			ed::PopStyleColor(2);
			ImGui::PopStyleVar();
		}
	}

	MaterialGraphView::~MaterialGraphView()
	{
		if (m_graphContext)
			ed::DestroyEditor(m_graphContext);
	}

	void MaterialGraphView::OnInvoke()
	{
		ed::Config config;
		config.SettingsFile = nullptr;
		m_graphContext = ed::CreateEditor(&config);
	}

	void MaterialGraphView::OnRender(const Engine::EngineFrame& context)
	{
		ed::SetCurrentEditor(m_graphContext);
		ed::Begin("MaterialGraphCanvas");

		if (m_firstFrame)
			PlaceNodes();

		const PinDesc textureInputs[] = { { 11, "UV", VectorPinColor } };
		const PinDesc textureOutputs[] = { { 12, "RGB", ColorPinColor }, { 13, "Alpha", FloatPinColor } };
		DrawBlueprintNode(1, "Texture Sample", IM_COL32(40, 90, 160, 255), textureInputs, textureOutputs);

		const PinDesc multiplyInputs[] = { { 21, "A", ColorPinColor }, { 22, "B", FloatPinColor } };
		const PinDesc multiplyOutputs[] = { { 23, "Out", ColorPinColor } };
		DrawSimpleNode(2, "MUL", multiplyInputs, multiplyOutputs);

		ed::BeginNode(ed::NodeId(3));
		ImGui::TextUnformatted("Scalar");
		ImGui::SetNextItemWidth(100.0f);
		ImGui::SliderFloat("##Value", &m_scalarValue, 0.0f, 1.0f);
		ImGui::SameLine();
		DrawPinRow({ 31, "Out", FloatPinColor }, ed::PinKind::Output);
		ed::EndNode();

		const PinDesc outputInputs[] = { { 41, "Base Color", ColorPinColor }, { 42, "Roughness", FloatPinColor }, { 43, "Metallic", FloatPinColor } };
		DrawBlueprintNode(4, "Material Output", IM_COL32(150, 60, 40, 255), outputInputs, {});

		DrawCommentNode(9, "Tree Section", ImVec2(220.0f, 200.0f));
		DrawVerticalNode(5, "Tree Root", 0, 52, TreeStyle);
		DrawVerticalNode(6, "Tree Child", 61, 0, TreeStyle);

		DrawVerticalNode(7, "Houdini A", 71, 72, HoudiniStyle);
		DrawVerticalNode(8, "Houdini B", 81, 82, HoudiniStyle);

		ed::Link(ed::LinkId(1001), ed::PinId(12), ed::PinId(21), ImColor(ColorPinColor));
		ed::Link(ed::LinkId(1002), ed::PinId(31), ed::PinId(22), ImColor(FloatPinColor));
		ed::Link(ed::LinkId(1003), ed::PinId(23), ed::PinId(41), ImColor(ColorPinColor), 2.0f);
		ed::Link(ed::LinkId(1004), ed::PinId(31), ed::PinId(42), ImColor(FloatPinColor));
		ed::Link(ed::LinkId(1005), ed::PinId(52), ed::PinId(61));
		ed::Link(ed::LinkId(1006), ed::PinId(72), ed::PinId(81));

		ed::End();

		if (m_firstFrame)
		{
			ed::NavigateToContent(0.0f);
			m_firstFrame = false;
		}

		ed::SetCurrentEditor(nullptr);
	}

	void MaterialGraphView::OnLibraryRegistered(const Engine::ReflectionLibrary& library)
	{
	}

	void MaterialGraphView::OnLibraryUnregistered(const Engine::ReflectionLibrary& library)
	{
	}
}