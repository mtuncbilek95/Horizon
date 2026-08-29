#include "QuatPropertyDrawer.h"

#include <Editor/Views/InspectorView/InspectorWidgets.h>
#include <Runtime/Math/Quat.h>

namespace Horizon::Editor
{
	namespace
	{
		constexpr f32 MinLengthSquared = 1.e-8f;
	}

	b8 QuatPropertyDrawer::OnDraw(const PropertyContext& context)
	{
		Math::Quat& rotation = context.pField->GetValueAs<Math::Quat>(context.pInstance);

		f32 values[4] = { rotation.X(), rotation.Y(), rotation.Z(), rotation.W() };

		InspectorWidgets::BeginRow(context.pLabel, context.pField);

		if (!InspectorWidgets::DrawVec4(context.pLabel, values))
			return false;

		const Math::Quat edited(values[0], values[1], values[2], values[3]);

		if (edited.SizeSquared() <= MinLengthSquared)
			return false;

		rotation = edited.GetNormalized();

		return true;
	}
}