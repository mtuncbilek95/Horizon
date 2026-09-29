#pragma once

#include <Editor/Components/PropertyDrawer.h>

namespace Horizon::Editor
{
	HCLASS();
	class H_EXPORT EnumPropertyDrawer final : public PropertyDrawer
	{
		HORIZON_TYPE_REFLECT(EnumPropertyDrawer);
	public:
		EnumPropertyDrawer() = default;
		~EnumPropertyDrawer() = default;

		Reflect::TypeHandle GetTargetType() const final { return Reflect::TypeOf<Reflect::EnumValue>(); }
		b8 OnDraw(const Reflect::Field& field, void* pValue, const PropertyContext& ctx) final;

	private:
		static i64 ReadUnderlying(const void* pValue, Reflect::TypeKind kind);
		static void WriteUnderlying(void* pValue, Reflect::TypeKind kind, i64 value);
	};
}