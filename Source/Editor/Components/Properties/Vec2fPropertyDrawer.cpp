#include "Vec2fPropertyDrawer.h"

#include <Editor/Components/Helpers/FloatWidget.h>
#include <Runtime/Math/Vec2f.h>

namespace Horizon::Editor
{
	b8 Vec2fPropertyDrawer::OnDraw(const Reflect::Field& field, void* pValue, const PropertyContext& ctx)
	{
		Math::Vec2f* pVec = static_cast<Math::Vec2f*>(pValue);

		f32 arr[2];
		pVec->Store(arr);

		if (!FloatWidget::Draw(field, arr, 2))
			return false;

		pVec->Set(arr[0], arr[1]);
		return true;
	}
}
