#include "Vec4fPropertyDrawer.h"

#include <Editor/Components/Helpers/FloatWidget.h>
#include <Runtime/Math/Vec4f.h>

namespace Horizon::Editor
{
	b8 Vec4fPropertyDrawer::OnDraw(const Reflect::Field& field, void* pValue, const PropertyContext& ctx)
	{
		Math::Vec4f* pVec = static_cast<Math::Vec4f*>(pValue);

		f32 arr[4];
		pVec->Store(arr);

		if (!FloatWidget::Draw(field, arr, 4))
			return false;

		pVec->Set(arr[0], arr[1], arr[2], arr[3]);
		return true;
	}
}
