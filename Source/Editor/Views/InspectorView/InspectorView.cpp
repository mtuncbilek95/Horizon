#include "InspectorView.h"

#include <Editor/Renderer/EditorContext.h>
#include <Editor/Models/SelectionModel.h>
#include <Editor/Attributes/HideInInspectorAttribute.h>
#include <Editor/Attributes/RangeAttribute.h>
#include <Editor/Attributes/TooltipAttribute.h>
#include <Editor/Views/InspectorView/InspectorWidgets.h>

#include <Engine/Core/Engine.h>
#include <Engine/Reflection/ReflectionSystem.h>
#include <Engine/World/ComponentTypeAttribute.h>
#include <Engine/World/ComponentObject.h>
#include <Engine/World/WorldService.h>

namespace Horizon::Editor
{
	namespace
	{
		b8 TryGetDataType(Reflect::TypeKind kind, ImGuiDataType& outType)
		{
			switch (kind)
			{
			case Reflect::TypeKind::Char:
			case Reflect::TypeKind::Signed8:
				outType = ImGuiDataType_S8;
				return true;
			case Reflect::TypeKind::Signed16:
				outType = ImGuiDataType_S16;
				return true;
			case Reflect::TypeKind::Signed32:
				outType = ImGuiDataType_S32;
				return true;
			case Reflect::TypeKind::Signed64:
				outType = ImGuiDataType_S64;
				return true;
			case Reflect::TypeKind::Unsigned8:
				outType = ImGuiDataType_U8;
				return true;
			case Reflect::TypeKind::Unsigned16:
				outType = ImGuiDataType_U16;
				return true;
			case Reflect::TypeKind::Unsigned32:
				outType = ImGuiDataType_U32;
				return true;
			case Reflect::TypeKind::Unsigned64:
				outType = ImGuiDataType_U64;
				return true;
			case Reflect::TypeKind::Float32:
				outType = ImGuiDataType_Float;
				return true;
			case Reflect::TypeKind::Float64:
				outType = ImGuiDataType_Double;
				return true;
			default:
				return false;
			}
		}
	}

	void InspectorView::OnInvoke()
	{
		m_currentWorld = GetContext()->pEngine->RequestService<Engine::WorldService>()->GetActiveWorld();
		m_drawerRegistry.BootstrapDrawers(GetContext()->pEngine);
	}

	void InspectorView::OnRender()
	{
		m_selected = GetContext()->pSelection->Get<Engine::EntityTag>();

		if (!m_currentWorld || !m_selected.IsValid())
			return;

		const f32 kButtonHeight = 30.0f;

		ImGui::BeginGroup();
		{
			List<Engine::ComponentObject*> comps;
			m_currentWorld->CollectComponents(m_selected, comps);

			for (Engine::ComponentObject* pComponent : comps)
				DrawComponent(pComponent);
		}
		ImGui::EndGroup();

		ImGui::Separator();

		ImGui::BeginGroup();
		{
			f32 width = ImGui::GetContentRegionAvail().x;
			f32 buttonWidth = width * 0.65f;

			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (width - buttonWidth) * 0.5f);

			if (ImGui::Button(ICON_FA_PLUS " Add Component", ImVec2(buttonWidth, kButtonHeight)))
			{
				m_popupPos.X() = ImGui::GetItemRectMin().x;
				m_popupPos.Y() = ImGui::GetItemRectMax().y + 4.0f;
				m_PopupWidth = buttonWidth;

				m_FocusSearch = true;

				ImGui::OpenPopup("##AddComponentPopup");
			}

			DrawAddComponentPopup();
		}
		ImGui::EndGroup();
	}

	void InspectorView::DrawComponent(Engine::ComponentObject* pComponent)
	{
		Reflect::Type* pType = GetContext()->pEngine->GetReflectionSystem()->GetType(pComponent->GetTypeId());

		if (!pType)
		{
			Terminal::Warn(StringOps::GetName(this), "{} component has no registered type", pComponent->GetTypeId().Index());
			return;
		}

		const c8* pHeader = pType->GetName().c_str();
		auto* pAttribute = pType->GetCustomAttribute<Engine::ComponentTypeAttribute>();

		if (pAttribute)
		{
			pHeader = pAttribute->GetComponentName().c_str();

			if (!pAttribute->GetVisibleOnSystem())
				return;
		}

		ImGui::PushID((i32)pComponent->GetTypeId().Index());

		if (ImGui::CollapsingHeader(pHeader, ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Spacing();

			if (ImGui::BeginTable("##fields", 2, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_PadOuterX))
			{
				ImGui::TableSetupColumn("##label", ImGuiTableColumnFlags_WidthFixed, 90.0f);
				ImGui::TableSetupColumn("##value", ImGuiTableColumnFlags_WidthStretch);

				DrawObject(pComponent, pType, 0);

				ImGui::EndTable();
			}

			ImGui::Spacing();
		}

		ImGui::PopID();
	}

	void InspectorView::DrawAddComponentPopup()
	{
		ImGui::SetNextWindowPos(ImVec2(m_popupPos.X(), m_popupPos.Y()));
		ImGui::SetNextWindowSize(ImVec2(m_PopupWidth, 380.0f));

		const b8 open = ImGui::BeginPopup("##AddComponentPopup");

		if (!open)
			return;

		ImGui::BeginChild("##ComponentList", ImVec2(0, 0), false);
		{
			auto* pReflect = GetContext()->pEngine->GetReflectionSystem();

			List<Reflect::Type*> types = pReflect->GetTypeByBase(Reflect::TypeOf<Engine::ComponentObject>());

			for (auto* type : types)
			{
				auto* pAttr = type->GetCustomAttribute<Engine::ComponentTypeAttribute>();

				if (pAttr == nullptr)
				{
					Terminal::Warn(StringOps::GetName(this), "{} type has no ComponentTypeAttribute", type->GetName());
					continue;
				}

				if (!pAttr->GetVisibleOnSystem())
					continue;

				if (ImGui::Selectable(pAttr->GetComponentName().c_str(), false, ImGuiSelectableFlags_AllowDoubleClick))
				{
					if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
					{
						Terminal::Warn(StringOps::GetName(this), "{} has been selected to be added!", pAttr->GetComponentName());
						ImGui::CloseCurrentPopup();
					}
				}
			}
		}

		ImGui::EndChild();
		ImGui::EndPopup();
	}

	b8 InspectorView::DrawObject(void* pInstance, Reflect::Type* pType, u32 depth)
	{
		b8 changed = false;

		for (const Reflect::Field& field : pType->GetFields())
		{
			if (field.GetCustomAttribute<HideInInspectorAttribute>())
				continue;

			PropertyContext context;
			context.pField = &field;
			context.pInstance = pInstance;
			context.pLabel = field.GetName().c_str();
			context.pEditState = &m_editState;
			context.depth = depth;

			ImGui::PushID((i32)field.GetOffset());
			changed |= DrawField(context);
			ImGui::PopID();
		}

		return changed;
	}

	b8 InspectorView::DrawField(const PropertyContext& context)
	{
		PropertyDrawer* pDrawer = m_drawerRegistry.Find(context.pField->GetTypeId());

		if (pDrawer)
			return pDrawer->OnDraw(context);

		if (context.pField->GetKind() != Reflect::TypeKind::Object)
			return DrawPrimitive(context);

		return DrawNested(context);
	}

	b8 InspectorView::DrawNested(const PropertyContext& context)
	{
		const Reflect::Field* pField = context.pField;

		void* pChild = pField->GetValue(context.pInstance);

		if (pField->GetMode() == Reflect::TypeMode::Pointer)
		{
			pChild = *static_cast<void**>(pChild);

			if (!pChild)
			{
				InspectorWidgets::BeginRow(context.pLabel, pField);
				ImGui::TextDisabled("None");

				return false;
			}
		}

		Reflect::Type* pType = GetContext()->pEngine->GetReflectionSystem()->GetType(pField->GetTypeId());

		if (!pType)
		{
			Terminal::Warn(StringOps::GetName(this), "{} field type is not registered", context.pLabel);
			return false;
		}

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);

		const b8 open = ImGui::TreeNodeEx(context.pLabel, ImGuiTreeNodeFlags_SpanAllColumns | ImGuiTreeNodeFlags_DefaultOpen);

		if (!open)
			return false;

		b8 changed = DrawObject(pChild, pType, context.depth + 1);
		ImGui::TreePop();

		return changed;
	}

	b8 InspectorView::DrawPrimitive(const PropertyContext& context)
	{
		const Reflect::Field* pField = context.pField;

		void* pValue = pField->GetValue(context.pInstance);
		Reflect::TypeKind kind = pField->GetKind();

		if (kind == Reflect::TypeKind::Enum)
			kind = pField->GetUnderlyingKind();

		InspectorWidgets::BeginRow(context.pLabel, pField);

		if (kind == Reflect::TypeKind::Boolean)
			return ImGui::Checkbox("##value", static_cast<b8*>(pValue));

		if (kind == Reflect::TypeKind::String)
		{
			std::string& text = pField->GetValueAs<std::string>(context.pInstance);

			c8 buffer[256];
			usize length = text.copy(buffer, sizeof(buffer) - 1);
			buffer[length] = '\0';

			if (!ImGui::InputText("##value", buffer, sizeof(buffer)))
				return false;

			text = buffer;

			return true;
		}

		ImGuiDataType dataType = ImGuiDataType_S32;

		if (!TryGetDataType(kind, dataType))
		{
			ImGui::TextDisabled("Unsupported");
			return false;
		}

		auto* pRange = pField->GetCustomAttribute<RangeAttribute>();

		if (pRange && dataType == ImGuiDataType_Float)
		{
			f32 minimum = pRange->GetMin();
			f32 maximum = pRange->GetMax();

			return ImGui::SliderScalar("##value", dataType, pValue, &minimum, &maximum);
		}

		if (pRange && dataType == ImGuiDataType_S32)
		{
			i32 minimum = (i32)pRange->GetMin();
			i32 maximum = (i32)pRange->GetMax();

			return ImGui::SliderScalar("##value", dataType, pValue, &minimum, &maximum);
		}

		f32 speed = 1.0f;

		if (dataType == ImGuiDataType_Float || dataType == ImGuiDataType_Double)
			speed = 0.01f;

		return ImGui::DragScalar("##value", dataType, pValue, speed);
	}
}