#pragma once

#include <Editor/Components/PropertyDrawer.h>
#include <Runtime/Containers/ListBase.h>

namespace Horizon::Reflect
{
	class Type;
}

namespace Horizon::Editor
{
	HCLASS();
	class EDITOR_API ListPropertyDrawer final : public PropertyDrawer
	{
		HORIZON_TYPE_REFLECT(ListPropertyDrawer);
	public:
		ListPropertyDrawer() = default;
		~ListPropertyDrawer() = default;

		Reflect::TypeHandle GetTargetType() const final { return Reflect::TypeOf<ListBase>(); }
		b8 OnDraw(const Reflect::Field& field, void* pValue, const PropertyContext& ctx) final;

	private:
		b8 DrawHeader(ListBase* pList, b8 readOnly);
		b8 DrawElements(const Reflect::Field& field, ListBase* pList, PropertyDrawer* pElementDrawer, const Reflect::Type* pElementType, const PropertyContext& ctx, b8 readOnly);
		b8 DrawElementRow(const Reflect::Field& field, ListBase* pList, usize index, PropertyDrawer* pElementDrawer, const Reflect::Type* pElementType, 
			const PropertyContext& ctx, b8 readOnly, i64& removeIndex);
	};
}