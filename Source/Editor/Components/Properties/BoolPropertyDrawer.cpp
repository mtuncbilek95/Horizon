#include "BoolPropertyDrawer.h"

#include <imgui.h>

namespace Horizon::Editor
{
	b8 BoolPropertyDrawer::OnDraw(const Reflect::Field& field, void* pValue, const PropertyContext& ctx)
	{
		b8* pBool = static_cast<b8*>(pValue);

		bool value = *pBool != 0;

		if (!ImGui::Checkbox("##value", &value))
			return false;

		*pBool = value;
		return true;
	}
}