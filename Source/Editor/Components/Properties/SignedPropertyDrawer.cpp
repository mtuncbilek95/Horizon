#include "SignedPropertyDrawer.h"

#include <Editor/Components/Helpers/IntegerWidget.h>

namespace Horizon::Editor
{
	b8 SignedPropertyDrawer::OnDraw(const Reflect::Field& field, void* pValue, const PropertyContext& ctx)
	{
		return IntegerWidget<i32, ImGuiDataType_S32>::Draw(field, static_cast<i32*>(pValue));
	}
}