#include "FloatPropertyDrawer.h"

#include <Editor/Components/Helpers/FloatWidget.h>

namespace Horizon::Editor
{
	b8 FloatPropertyDrawer::OnDraw(const Reflect::Field& field, void* pValue, const PropertyContext& ctx)
	{
		f32* pFloat = static_cast<f32*>(pValue);

		f32 value = *pFloat;

		if (!FloatWidget::Draw(field, &value, 1))
			return false;

		*pFloat = value;
		return true;
	}
}