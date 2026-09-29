#include "Vec3fPropertyDrawer.h"

#include <Editor/Components/Properties/FloatWidget.h>
#include <Runtime/Math/Vec3f.h>

namespace Horizon::Editor
{
	b8 Vec3fPropertyDrawer::OnDraw(const Reflect::Field& field, void* pValue, const PropertyContext& ctx)
	{
		Math::Vec3f* pVec = static_cast<Math::Vec3f*>(pValue);

		f32 arr[3];
		pVec->Store(arr);

		if (!FloatWidget::Draw(field, arr, 3))
			return false;

		pVec->Set(arr[0], arr[1], arr[2]);
		return true;
	}
}
