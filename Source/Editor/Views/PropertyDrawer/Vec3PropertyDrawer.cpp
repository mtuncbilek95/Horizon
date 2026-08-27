#include "Vec3PropertyDrawer.h"

#include <Editor/Views/InspectorView/InspectorWidgets.h>

namespace Horizon::Editor
{
	b8 Vec3PropertyDrawer::OnDraw(const PropertyContext& context)
	{
		Math::Vec3f& vector = context.pField->GetValueAs<Math::Vec3f>(context.pInstance);

		f32 values[3] = { vector.X(), vector.Y(), vector.Z() };

		InspectorWidgets::BeginRow(context.pLabel, context.pField);

		if (!InspectorWidgets::DrawVec3(context.pLabel, values))
			return false;

		vector = Math::Vec3f(values[0], values[1], values[2]);

		return true;
	}
}