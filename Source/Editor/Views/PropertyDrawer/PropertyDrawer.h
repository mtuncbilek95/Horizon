#pragma once

#include <Editor/Views/PropertyDrawer/PropertyContext.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Editor
{
	class H_EXPORT PropertyDrawer : public Reflect::Base
	{
	public:
		virtual ~PropertyDrawer() = default;

		virtual Reflect::TypeHandle GetTargetType() const = 0;
		virtual b8 OnDraw(const PropertyContext& context) = 0;
	};
}