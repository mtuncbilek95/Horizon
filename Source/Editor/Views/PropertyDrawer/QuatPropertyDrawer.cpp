#include "QuatPropertyDrawer.h"

#include <Editor/Views/InspectorView/InspectorWidgets.h>
#include <Runtime/Math/Vec3f.h>

namespace Horizon::Editor
{
	b8 QuatPropertyDrawer::OnDraw(const PropertyContext& context)
	{
		Math::Quat& rotation = context.pField->GetValueAs<Math::Quat>(context.pInstance);
		EditState& state = *context.pEditState;

		const ImGuiID id = ImGui::GetID(context.pLabel);

		f32 values[3];

		if (state.IsActive(id))
		{
			values[0] = state.buffer[0];
			values[1] = state.buffer[1];
			values[2] = state.buffer[2];
		}
		else
		{
			Math::Vec3f euler = rotation.Euler();
			values[0] = euler.X();
			values[1] = euler.Y();
			values[2] = euler.Z();
		}

		InspectorWidgets::BeginRow(context.pLabel, context.pField);

		b8 active = false;
		b8 changed = InspectorWidgets::DrawVec3(context.pLabel, values, &active);

		if (active)
			state.Capture(id, values, 3);
		else if (state.IsActive(id))
			state.Release();

		if (!changed)
			return false;

		rotation = Math::Quat::MakeFromEuler(Math::Vec3f(values[0], values[1], values[2]));

		return true;
	}
}