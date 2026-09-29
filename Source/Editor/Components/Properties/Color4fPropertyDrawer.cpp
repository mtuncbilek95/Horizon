#include "Color4fPropertyDrawer.h"

#include <Runtime/Math/Color4f.h>

#include <imgui.h>

namespace Horizon::Editor
{
	b8 Color4fPropertyDrawer::OnDraw(const Reflect::Field& field, void* pValue, const PropertyContext& ctx)
	{
		Math::Color4f* pColor = static_cast<Math::Color4f*>(pValue);

		f32 arr[4];
		pColor->Store(arr);

		ImGui::SetNextItemWidth(-FLT_MIN);

		if (!ImGui::ColorEdit4("##value", arr, ImGuiColorEditFlags_DisplayHex | ImGuiColorEditFlags_AlphaPreviewHalf))
			return false;

		pColor->Set(arr[0], arr[1], arr[2], arr[3]);
		return true;
	}
}