#include "UnsignedPropertyDrawer.h"

#include <Editor/Components/Helpers/IntegerWidget.h>

namespace Horizon::Editor
{
	b8 UnsignedPropertyDrawer::OnDraw(const Reflect::Field& field, void* pValue, const PropertyContext& ctx)
	{
		return IntegerWidget<u32, ImGuiDataType_U32>::Draw(field, static_cast<u32*>(pValue));
	}
}